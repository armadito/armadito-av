#ifndef UPDATEINFOMODEL_H
#define UPDATEINFOMODEL_H

#include <libuhuru/core.h>
#include <QObject>

class UpdateInfoModel : public QObject {
    Q_OBJECT

public:
    UpdateInfoModel();
    struct uhuru_info *RefreshUpdateInfo();

private:
    struct uhuru_info *RetrieveUpdateInfo(int use_daemon);
    struct uhuru_info *Uinfo;
};

#endif // UPDATEINFOMODEL_H
