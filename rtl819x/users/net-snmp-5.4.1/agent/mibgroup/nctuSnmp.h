/* This file was generated by mib2c and is intended for use as a mib module
  for the ucd-snmp snmpd agent. */


#ifndef _MIBGROUP_NCTUSNMP_H
#define _MIBGROUP_NCTUSNMP_H


/* we may use header_generic and header_simple_table from the util_funcs module */


config_require(util_funcs)


/* function prototypes */


void   init_nctuSnmp(void);
FindVarMethod var_nctuSnmp;

WriteMethod write_reinitialization;
WriteMethod write_submit;
WriteMethod write_rwCommunity;
WriteMethod write_roCommunity;



#endif /* _MIBGROUP_NCTUSNMP_H */
