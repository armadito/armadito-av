#include "updateinfomodel.h"

struct uhuru_info *UpdateInfoModel::RetrieveUpdateInfo(int use_daemon)
{
     struct uhuru *u;
     struct uhuru_info *info;

     u = uhuru_open(NULL);
     info = uhuru_info_new(u);

     //info_to_stdout(info);

     uhuru_close(u, NULL);
     return info;
}

struct uhuru_info *UpdateInfoModel::RefreshUpdateInfo()
{ 
     Uinfo = RetrieveUpdateInfo((int)1);
     return Uinfo;
}
	
UpdateInfoModel::UpdateInfoModel()
{

}




