%module(package="openorbit") config

%{
  /* Code for wrapper */
  #include "settings.h"
%}

/*
  Wrapped functions
*/


%rename(GetBool) ooConfGetBoolDef;
int ooConfGetBoolDef(const char *key, bool *val, bool defVal);

%rename(GetInt) ooConfGetIntDef;
int ooConfGetIntDef(const char *key, int *val, int defVal);

%rename(GetFloat) ooConfGetFloatDef;
int ooConfGetFloatDef(const char *key, float *val, float defVal);

%rename(GetStr) ooConfGetStrDef;
int ooConfGetStrDef(const char *key, const char **val, const char *defVal);
