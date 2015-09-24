#include "updateinfomodel.h"

struct uhuru_info *UpdateInfoModel::RetrieveUpdateInfo(int use_daemon)
{
     struct uhuru *u;
     struct uhuru_info *info;

     u = uhuru_open(use_daemon);
     info = uhuru_info_new(u);

     //info_to_stdout(info);

     uhuru_close(u);

     //emit UpdateInfoChanged(&info);

     return info;
}

void UpdateInfoModel::freeUpdateInfo()
{
     uhuru_info_free(Uinfo);     	
}	

struct uhuru_info *UpdateInfoModel::getUpdateInfo()
{ 
     return Uinfo; 
}

UpdateInfoModel::UpdateInfoModel()
{
     // Retrieve info from libuhuru, using daemon
     Uinfo = RetrieveUpdateInfo((int)1);
}




