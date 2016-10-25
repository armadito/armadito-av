struct a6o_info {
	enum a6o_update_status global_status;
	time_t global_update_ts;
	/* NULL terminated array of pointers to struct a6o_module_info */
	struct a6o_module_info **module_infos;
};

const char *a6o_update_status_str(enum a6o_update_status status);

struct a6o_info *a6o_info_new(struct armadito *armadito);

void a6o_info_to_stdout(struct a6o_info *info);

void a6o_info_free(struct a6o_info *info);

