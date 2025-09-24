#include "projectmodel.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QMessageBox>

ProjectModel::ProjectModel(QObject *parent) : QObject(parent)
{
    createdTime = QDateTime::currentDateTime();
    lastModified = createdTime;
}

bool ProjectModel::loadProject(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开项目文件:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "项目文件格式错误:" << filePath;
        return false;
    }

    QJsonObject root = doc.object();

    projectName = root["projectName"].toString();
    createdTime = QDateTime::fromString(root["createdTime"].toString(), Qt::ISODate);
    lastModified = QDateTime::fromString(root["lastModified"].toString(), Qt::ISODate);

    // 加载设计线
    m_designLinesFile.clear();
    QJsonArray designLinesArray = root["designLines"].toArray();
    for (const auto& item : designLinesArray) {
        m_designLinesFile.append(DesignLineFile::fromJson(item.toObject()));
    }

    // 加载架次
    m_batches.clear();
    QJsonArray batchesArray = root["batches"].toArray();
    for (const auto& item : batchesArray) {
        m_batches.append(Batch::fromJson(item.toObject()));
    }

    projectPath = filePath;
    emit projectModified();
    emit designLinesChanged();
    emit batchesChanged();

    return true;
}

bool ProjectModel::saveProject(const QString& filePath) {
    if (filePath.isEmpty() && projectPath.isEmpty()) {
        return false;
    }

    QString savePath = filePath.isEmpty() ? projectPath : filePath;
    lastModified = QDateTime::currentDateTime();

    QJsonObject root;
    root["projectName"] = projectName;
    root["createdTime"] = createdTime.toString(Qt::ISODate);
    root["lastModified"] = lastModified.toString(Qt::ISODate);

    // 保存设计线
    QJsonArray designLinesArray;
    for (const auto& dlf : m_designLinesFile) {
        designLinesArray.append(dlf.toJson());
    }
    root["designLines"] = designLinesArray;

    // 保存批次
    QJsonArray batchesArray;
    for (const auto& batch : m_batches) {
        batchesArray.append(batch.toJson());
    }
    root["batches"] = batchesArray;

    QJsonDocument doc(root);
    QFile file(savePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法保存项目文件:" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    if (projectPath.isEmpty()) {
        projectPath = savePath;
    }

    emit projectModified();
    return true;
}


bool ProjectModel::validateProject() const
{
    // 检查所有数据文件是否存在
//    for (const QString& filePath : dataFiles) {
//        QString absolutePath = filePath;
//        if (QFileInfo(filePath).isRelative()) {
//            absolutePath = QDir(projectPath).absoluteFilePath(filePath);
//        }

//        if (!QFile::exists(absolutePath)) {
//            qDebug() << "数据文件不存在:" << absolutePath;
//            return false;
//        }
//    }
    for (const DesignLineFile& file : m_designLinesFile){
        QString absolutePath = file.filePath;
        if (QFileInfo(absolutePath).isRelative()){
            absolutePath = QDir(projectPath).absoluteFilePath(absolutePath);
        }
        if(!QFile::exists(absolutePath)){
            qDebug() << "数据文件不存在：" << absolutePath;
            return false;
        }
    }
    return true;
}


// DataPoint测点结构实现
QJsonObject DataPoint::toJson() const {
    QJsonObject json;
    json["LINE"] = lineId;
    json["FN"] = QString("%1").arg(fn, 6, 10, QChar('0'));
    json["X"] = coordinate.x();
    json["Y"] = coordinate.y();
    json["RALT"] = alt;
    json["Visible"] = int(isVisible);
    return json;
}

DataPoint DataPoint::fromJson(const QJsonObject& json) {
    DataPoint point;
    point.lineId = json["LINE"].toString();
    point.fn = json["FN"].toString().toInt();
    point.coordinate = QPointF(json["X"].toDouble(), json["Y"].toDouble());
    point.alt = json["RALT"].toDouble();
    point.isVisible = bool(json["Visible"].toInt());
    return point;
}

// DesignLine设计线段结构实现
QJsonObject DesignLine::toJson() const {
    QJsonObject json;
    json["Line"] = lineName;
    json["X1"] = x1;
    json["Y1"] = y1;
    json["X2"] = x2;
    json["Y2"] = y2;
    json["MatchTimes"] = matchTimes;
    return json;
}

DesignLine DesignLine::fromJson(const QJsonObject& json) {
    DesignLine dl;
    dl.lineName = json["Line"].toString();
    dl.x1 = json["X1"].toDouble();
    dl.y1 = json["Y1"].toDouble();
    dl.x2 = json["X2"].toDouble();
    dl.y2 = json["Y2"].toDouble();
    dl.matchTimes = json["MatchTimes"].toInt();
    return dl;
}

// DesignLineFile结构实现
QJsonObject DesignLineFile::toJson() const {
    QJsonObject json;
    json["filePath"] = filePath;

    QJsonArray dataArray;
    for (const auto& point : data) {
        dataArray.append(point.toJson());
    }
    json["data"] = dataArray;
    json["visible"] = visible;

    return json;
}

DesignLineFile DesignLineFile::fromJson(const QJsonObject& json) {
    DesignLineFile dlf;
    dlf.filePath = json["filePath"].toString();
    dlf.visible = json["visible"].toBool(true);

    QJsonArray dataArray = json["data"].toArray();
    for (const auto& item : dataArray) {
        dlf.data.append(DesignLine::fromJson(item.toObject()));
    }

    return dlf;
}

// Batch结构实现
QJsonObject Batch::toJson() const {
    QJsonObject json;
    json["batchName"] = batchName;

    QJsonArray filesArray;
    for (const auto& file : filePaths) {
        filesArray.append(file);
    }
    json["fileNames"] = filesArray;

    QJsonArray pointsArray;
    for (const auto& point : points) {
        pointsArray.append(point.toJson());
    }
    json["points"] = pointsArray;

    QJsonArray linesArray;
    for (const auto& line : relatedLines) {
        linesArray.append(line);
    }
    json["relatedLines"] = linesArray;

    json["fileLineCount"] = size;

    return json;
}

Batch Batch::fromJson(const QJsonObject& json) {
    Batch batch;
    batch.batchName = json["batchName"].toString();
    batch.size = json["fileLineCount"].toInt(0);

    QJsonArray filesArray = json["fileNames"].toArray();
    for (const auto& item : filesArray) {
        batch.filePaths.append(item.toString());
    }

    QJsonArray pointsArray = json["points"].toArray();
    for (const auto& item : pointsArray) {
        batch.points.append(DataPoint::fromJson(item.toObject()));
    }

    QJsonArray linesArray = json["relatedLines"].toArray();
    for (const auto& item : linesArray) {
        batch.relatedLines.append(item.toString());
    }

    return batch;
}

bool ProjectModel::addDesignLineFile(const QString& filePath) {
//    QFileInfo fileInfo(filePath);
//    QString fileName = fileInfo.fileName();

    // 检查文件是否已添加
    for (const auto& dlf : m_designLinesFile) {
        if (dlf.filePath == filePath) {
            QMessageBox::warning(nullptr, "重复导入",
                                 QString("设计线文件 [%1] 已经导入过了！").arg(filePath));
            return false;
        }
    }

    // 解析文件内容到data
    DesignLineFile newFile;
    newFile.filePath = filePath;
    newFile.visible = true;

    // 打开文件并解析内容
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        // 文件格式为：Line, X, Y, ,..., point, length
        // 跳过表头行
        if (!in.atEnd()) in.readLine();

        while (!in.atEnd()) {
            QString line1 = in.readLine().trimmed();
            QString line2 = in.readLine().trimmed();
            if (line1.isEmpty() || line2.isEmpty()) continue;

            // 按空格分割数据
            QStringList parts1 = line1.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            QStringList parts2 = line2.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (parts1.size() >= 3) {
                DesignLine dl;
                dl.lineName = parts1[0].trimmed();
                dl.x1 = parts1[1].trimmed().toDouble();
                dl.y1 = parts1[2].trimmed().toDouble();
                dl.x2 = parts2[1].trimmed().toDouble();
                dl.y2 = parts2[2].trimmed().toDouble();
                dl.matchTimes = 0;

                newFile.data.append(dl);
            }
        }
        file.close();
    } else {
        // 文件打开失败，仍添加文件但数据为空
        qWarning() << "无法打开设计线文件:" << filePath;
    }

    m_designLinesFile.append(newFile);

    lastModified = QDateTime::currentDateTime();
    emit designLinesChanged();
    emit projectModified();
    return true;
}

bool ProjectModel::removeDesignLineFile(int index) {
    if (index >= 0 && index < m_designLinesFile.size()) {
        m_designLinesFile.removeAt(index);
        lastModified = QDateTime::currentDateTime();
        emit designLinesChanged();
        emit projectModified();
        return true;
    }
    return false;
}

void ProjectModel::setDesignLineVisibility(int index, bool visible) {
    if (index >= 0 && index < m_designLinesFile.size()) {
        m_designLinesFile[index].visible = visible;
        lastModified = QDateTime::currentDateTime();
        emit designLinesChanged();
        emit projectModified();
    }
}

bool ProjectModel::addBatch(const QString& batchName) {
    // 检查批次名是否已存在
    for (const auto& batch : m_batches) {
        if (batch.batchName == batchName) {
            QMessageBox::warning(nullptr, "重复添加",
                                 QString("架次 [%1] 已经添加过了！").arg(batchName));
            return false;
        }
    }

    Batch newBatch;
    newBatch.batchName = batchName;
    newBatch.size = 0; // 初始化为0，添加第一个文件时设置

    m_batches.append(newBatch);

    lastModified = QDateTime::currentDateTime();
    emit batchesChanged();
    emit projectModified();
    return true;
}

bool ProjectModel::removeBatch(int index) {
    if (index >= 0 && index < m_batches.size()) {
        m_batches.removeAt(index);
        lastModified = QDateTime::currentDateTime();
        emit batchesChanged();
        emit projectModified();
        return true;
    }
    return false;
}

bool ProjectModel::addDataFile(int batchIndex, const QString& filePath) {
    if (batchIndex < 0 || batchIndex >= m_batches.size()) {
        return false;
    }

//    QFileInfo fileInfo(filePath);
//    QString fileName = fileInfo.fileName();

    Batch& batch = m_batches[batchIndex];

    // 检查文件是否已添加到该批次
    for (const auto& f : batch.filePaths) {
        if (f == filePath) {
            QMessageBox::warning(nullptr, "重复导入",
                                 QString("文件 [%1] 已经导入过了！").arg(filePath));
            return false;
        }
    }

    // 检查行数约束
    if (batch.size == 0) {
        // 第一个文件，设置行数约束，解析坐标到Datapoints和relatedLines

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(nullptr, "错误", "无法打开文件 ");
            return false;
        }
        QTextStream in(&file);
        in.setCodec("UTF-8");
        QStringList previewData;
        QString lastLine;

        // 跳过头部
        while (!in.atEnd()) {
            lastLine = in.readLine().trimmed();
            if (lastLine.split(' ', QString::SkipEmptyParts)[0] == "LINE")
                break;
        }
        while (!in.atEnd()){
            lastLine = in.readLine().trimmed();
            QStringList parts = lastLine.split(' ', QString::SkipEmptyParts);
            if (parts.size() >= 8) {
                DataPoint dp;
                dp.lineId = parts[0].trimmed();
                dp.fn = parts[1].trimmed().toInt();
                dp.coordinate = QPointF(parts[4].trimmed().toDouble(),
                        parts[5].trimmed().toDouble());
                dp.alt = parts[7].trimmed().toDouble();
                dp.isVisible = true;
                batch.points.append(dp);
                batch.size ++;
            }
        }
        file.close();
    } else {
        // 非本架次第一个文件，只检查行数约束，添加文件名，不添加点数据
        // 获取文件行数
        int size = getSize(filePath);
        if (size <= 0) {
            qDebug() << "文件为空或无法读取";
            return false;
        }
        if (batch.size != size) {
            // 行数不匹配，返回false
            QMessageBox::warning(nullptr, "大小不匹配",
                             QString("文件 [%1] 与已有文件 [%2] 大小不匹配").arg(filePath, batch.filePaths[0]));
        return false;
        }
    }


    batch.filePaths.append(filePath);

    lastModified = QDateTime::currentDateTime();
    emit batchesChanged();
    emit projectModified();
    return true;
}

bool ProjectModel::removeDataFile(int batchIndex, int fileIndex) {
    if (batchIndex < 0 || batchIndex >= m_batches.size()) {
        return false;
    }

    Batch& batch = m_batches[batchIndex];

    if (fileIndex < 0 || fileIndex >= batch.filePaths.size()) {
        return false;
    }

    batch.filePaths.removeAt(fileIndex);

    // 如果批次中已没有文件，重置行数约束
    if (batch.filePaths.isEmpty()) {
        batch.size = 0;
        batch.points.clear();
    }

    lastModified = QDateTime::currentDateTime();
    emit batchesChanged();
    emit projectModified();
    return true;
}

int ProjectModel::getSize(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }

    int size = 0;
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString lastLine = in.readLine().trimmed();
        if (lastLine.split(' ', QString::SkipEmptyParts)[0] == "LINE")
            break;
    }

    while (!in.atEnd()) {
        in.readLine();
        size++;
    }

    file.close();
    return size;
}
//QJsonObject ProjectModel::toJson() const
//{
//    QJsonObject json;
//    json["projectName"] = projectName;
//    json["description"] = description;
//    json["createdTime"] = createdTime.toString(Qt::ISODate);
//    json["lastModified"] = lastModified.toString(Qt::ISODate);

//    QJsonArray filesArray;
//    for (const QString& file : dataFiles) {
//        filesArray.append(file);
//    }
//    json["dataFiles"] = filesArray;

//    json["commonSettings"] = commonSettings;

//    json["designLines"] = designObj;

//    return json;
//}

//void ProjectModel::fromJson(const QJsonObject& json)
//{
//    projectName = json["projectName"].toString();
//    description = json["description"].toString();
//    createdTime = QDateTime::fromString(json["createdTime"].toString(), Qt::ISODate);
//    lastModified = QDateTime::fromString(json["lastModified"].toString(), Qt::ISODate);

//    dataFiles.clear();
//    QJsonArray filesArray = json["dataFiles"].toArray();
//    for (const QJsonValue& value : filesArray) {
//        dataFiles.append(value.toString());
//    }

//    designFiles.clear();
//    if (json.contains("designLines") && json["designLines"].isObject()) {
//        QJsonObject designLines = json["designLines"].toObject();
//        designObj = designLines;

//        if (designLines.contains("fileName") && designLines["fileName"].isString()) {
//            QString fileName = designLines["fileName"].toString();
//            designFiles.append(fileName);
//        }
//    }
//    qDebug() << designFiles;
//    commonSettings = json["commonSettings"].toObject();
//}



//void ProjectModel::addDataFile(const QString& filePath)
//{
//    if (!dataFiles.contains(filePath)) {
//        dataFiles.append(filePath);
//    }
//}

//void ProjectModel::removeDataFile(const QString& filePath)
//{
//    dataFiles.removeAll(filePath);
//}

//bool ProjectModel::hasDataFile(const QString& filePath) const
//{
//    return dataFiles.contains(filePath);
//}

//bool ProjectModel::hasDesignFile(const QString &filePath) const
//{
//    return designFiles.contains(filePath);
//}

//void ProjectModel::setCommonSetting(const QString& key, const QJsonValue& value)
//{
//    commonSettings[key] = value;
//}

//QJsonValue ProjectModel::getCommonSetting(const QString& key) const
//{
//    return commonSettings.value(key);
//}


//bool ProjectModel::saveProject(const QString& filePath)
//{
//    QFile file(filePath);
//    if (!file.open(QIODevice::WriteOnly)) {
//        qDebug() << "无法保存项目文件:" << filePath;
//        return false;
//    }

//    lastModified = QDateTime::currentDateTime();
//    projectPath = QFileInfo(filePath).absolutePath();

//    QJsonDocument doc(toJson());
//    file.write(doc.toJson());

//    return true;
//}

//bool ProjectModel::loadProject(const QString& filePath)
//{
//    QFile file(filePath);
//    if (!file.open(QIODevice::ReadOnly)) {
//        qDebug() << "无法打开项目文件:" << filePath;
//        return false;
//    }

//    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
//    if (doc.isNull() || !doc.isObject()) {
//        qDebug() << "项目文件格式错误:" << filePath;
//        return false;
//    }

//    fromJson(doc.object());
//    projectPath = QFileInfo(filePath).absolutePath();

//    return validateProject();
//}
