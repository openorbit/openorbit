%module(package="openorbit") config

%{
  /* Code for wrapper */
  #include "settings.h"
%}

/*
  Wrapped functions
*/


%rename(GetBool) config_get_bool_def;
int config_get_bool_def(const char *key, bool *val, bool defVal);

%rename(GetInt) config_get_int_def;
int config_get_int_def(const char *key, int *val, int defVal);

%rename(GetFloat) config_get_float_def;
int config_get_float_def(const char *key, float *val, float defVal);

%rename(GetStr) config_get_str_def;
int config_get_str_def(const char *key, const char **val, const char *defVal);
