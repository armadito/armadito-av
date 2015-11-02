#ifndef UPDATEINFOMODEL_H
#define UPDATEINFOMODEL_H

#include <QObject>

class UpdateInfoModel : public QObject {
    Q_OBJECT

public:
    UpdateInfoModel();
#if 0
    struct uhuru_info *RefreshUpdateInfo();
#endif

private:
#if 0
    struct uhuru_info *RetrieveUpdateInfo(int use_daemon);
    struct uhuru_info *Uinfo;
#endif
};

#endif // UPDATEINFOMODEL_H
