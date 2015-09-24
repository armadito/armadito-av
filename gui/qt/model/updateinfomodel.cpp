#include "updateinfomodel.h"

UpdateInfoModel::UpdateInfoModel()
{
     struct uhuru *u;
     struct uhuru_info *info;
     struct uhuru_module_info **m;
     int use_daemon = 1;

     u = uhuru_open(use_daemon);
     info = uhuru_info_new(u);

     if (info->module_infos != NULL) {
      for(m = info->module_infos; *m != NULL; m++){
        fprintf(stdout, "Module %s !\n", (*m)->name );
        fprintf(stdout, "Update date %s !\n", (*m)->update_date );
      }
     }

     //fprintf(stdout, "UhuruInfoModel !\n");

     uhuru_info_free(info);
     uhuru_close(u);
}
