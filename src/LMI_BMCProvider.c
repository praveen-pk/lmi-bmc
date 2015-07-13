#include <konkret/konkret.h>
#include "LMI_BMC.h"
#include "LmiBmc.h"
#include <string.h>

extern int bmc_max_ips, bmc_max_protos;

static const CMPIBroker* _cb = NULL;

static void LMI_BMCInitialize()
{

/*
 * Based on the system Vendor, set the global max variables.
 */
    set_bmc_max_vars();

}

static CMPIStatus LMI_BMCCleanup(
    CMPIInstanceMI* mi,
    const CMPIContext* cc,
    CMPIBoolean term)
{
    CMReturn(CMPI_RC_OK);
}

static CMPIStatus LMI_BMCEnumInstanceNames(
    CMPIInstanceMI* mi,
    const CMPIContext* cc,
    const CMPIResult* cr,
    const CMPIObjectPath* cop)
{
    return KDefaultEnumerateInstanceNames(
        _cb, mi, cc, cr, cop);
}

static CMPIStatus LMI_BMCEnumInstances(
    CMPIInstanceMI* mi,
    const CMPIContext* cc,
    const CMPIResult* cr,
    const CMPIObjectPath* cop,
    const char** properties)
{
    const char *ns = KNameSpace(cop);
    CMPIString *x = lmi_get_system_creation_class_name();
    BMC_info *bmc_info = NULL;
    int i=0;
    char *vendor = NULL;
    char *errstr = NULL;

    vendor = get_bios_vendor();
    if (vendor == NULL)
    {
	lmi_error("Not able to determine the System Vendor");
	asprintf (&errstr,"Not able to determine the System Vendor\n" );
	goto failed;
    }

    bmc_info = (BMC_info *) alloc_init_bmc_info ();
    if (bmc_info == NULL)
    {
	lmi_error("Allocation of BMC Info failed.");
	goto failed;
    }

    if ( does_vendor_support_ipmi (vendor) )
    {
	if (populate_bmc_info_with_ipmi(bmc_info)){
	    /*bmc_info is already de-allocated, set it to NULL*/
	    bmc_info=NULL;
	    asprintf(&errstr,"Failed running the ipmitool command. Check if ipmi service is running");
	    goto failed;
	}
    }
    else
    {
	/*Fallback to interfaces other than IPMI here*/
	asprintf(&errstr,"BIOS vendor: %s is NOT SUPPORTED", vendor);
	goto failed;
    }

    LMI_BMC inst;
    LMI_BMC_Init(&inst,_cb,ns );
    /*TODO: CIM_LogicalDevice features */
    LMI_BMC_Set_SystemCreationClassName (&inst,lmi_get_system_creation_class_name());
    LMI_BMC_Init_IP4Addresses(&inst,bmc_max_ips);
    for (i=0;i<bmc_max_ips;i++)
    {
	LMI_BMC_Set_IP4Addresses (&inst, i, strdup(bmc_info->IP4Addresses[i]));
    }

    LMI_BMC_Init_IP4Netmasks(&inst,bmc_max_ips);

    for (i=0;i<bmc_max_ips;i++)
    {
	LMI_BMC_Set_IP4Netmasks(&inst, i, strdup(bmc_info->IP4Netmasks[i]));
    }

    LMI_BMC_Set_IP4AddressSource(&inst,strdup(bmc_info->IP4AddressSource));


    LMI_BMC_Init_BMC_URLs(&inst,1);
    LMI_BMC_Set_BMC_URLs(&inst,0,strdup(bmc_info->BMC_URLs[0]));
    LMI_BMC_Set_PermanentMACAddress(&inst, strdup(bmc_info->PermanentMACAddress));
    LMI_BMC_Set_FirmwareVersion(&inst, strdup(bmc_info->FirmwareVersion));

    /*Set the List of Supported Protocols here. First one will be IPMI. Later on also capture WSMAN if possible*/
    LMI_BMC_Init_SupportedProtos(&inst,bmc_max_protos);
    LMI_BMC_Init_SupportedProtoVersions(&inst,bmc_max_protos);

    for (i=0; i<bmc_max_protos; i++)
    {
	LMI_BMC_Set_SupportedProtos(&inst, i, strdup(bmc_info->supportedProtos[i]));
	LMI_BMC_Set_SupportedProtoVersions(&inst, i, strdup(bmc_info->supportedProtoVersions[i]));
    }

    free_bmc_info(bmc_info);
    free (vendor);
    KReturnInstance(cr, inst);
    CMReturn(CMPI_RC_OK);


failed:
    free (vendor);
    free_bmc_info(bmc_info);
//TODO: Return an empty instance.
    if (errstr){
	CMReturnWithChars(_cb, CMPI_RC_ERR_FAILED, errstr);
    }
    else
    {
	CMReturn(CMPI_RC_ERROR);
    }
}

static CMPIStatus LMI_BMCGetInstance(
    CMPIInstanceMI* mi,
    const CMPIContext* cc,
    const CMPIResult* cr,
    const CMPIObjectPath* cop,
    const char** properties)
{
    return KDefaultGetInstance(
        _cb, mi, cc, cr, cop, properties);
}

static CMPIStatus LMI_BMCCreateInstance(
    CMPIInstanceMI* mi,
    const CMPIContext* cc,
    const CMPIResult* cr,
    const CMPIObjectPath* cop,
    const CMPIInstance* ci)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

static CMPIStatus LMI_BMCModifyInstance(
    CMPIInstanceMI* mi,
    const CMPIContext* cc,
    const CMPIResult* cr,
    const CMPIObjectPath* cop,
    const CMPIInstance* ci,
    const char** properties)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

static CMPIStatus LMI_BMCDeleteInstance(
    CMPIInstanceMI* mi,
    const CMPIContext* cc,
    const CMPIResult* cr,
    const CMPIObjectPath* cop)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

static CMPIStatus LMI_BMCExecQuery(
    CMPIInstanceMI* mi,
    const CMPIContext* cc,
    const CMPIResult* cr,
    const CMPIObjectPath* cop,
    const char* lang,
    const char* query)
{
    CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}

CMInstanceMIStub(
    LMI_BMC,
    LMI_BMC,
    _cb,
    LMI_BMCInitialize())

static CMPIStatus LMI_BMCMethodCleanup(
    CMPIMethodMI* mi,
    const CMPIContext* cc,
    CMPIBoolean term)
{
    CMReturn(CMPI_RC_OK);
}

static CMPIStatus LMI_BMCInvokeMethod(
    CMPIMethodMI* mi,
    const CMPIContext* cc,
    const CMPIResult* cr,
    const CMPIObjectPath* cop,
    const char* meth,
    const CMPIArgs* in,
    CMPIArgs* out)
{
    return LMI_BMC_DispatchMethod(
        _cb, mi, cc, cr, cop, meth, in, out);
}

CMMethodMIStub(
    LMI_BMC,
    LMI_BMC,
    _cb,
    LMI_BMCInitialize())

KUint32 LMI_BMC_RequestStateChange(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    const KUint16* RequestedState,
    KRef* Job,
    const KDateTime* TimeoutPeriod,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KUint32 LMI_BMC_SetPowerState(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    const KUint16* PowerState,
    const KDateTime* Time,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KUint32 LMI_BMC_Reset(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KUint32 LMI_BMC_EnableDevice(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    const KBoolean* Enabled,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KUint32 LMI_BMC_OnlineDevice(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    const KBoolean* Online,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KUint32 LMI_BMC_QuiesceDevice(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    const KBoolean* Quiesce,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KUint32 LMI_BMC_SaveProperties(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KUint32 LMI_BMC_RestoreProperties(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KUint32 LMI_BMC_set_IP4Address(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    const KStringA* IP4Addresses,
    const KStringA* Netmasks,
    const KString* Gateway,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KUint32 LMI_BMC_set_IP6Address(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    const KStringA* IP6Addresses,
    const KStringA* Netmasks,
    const KString* Gateway,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KUint32 LMI_BMC_set_VLAN(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    const KString* Vlan,
    CMPIStatus* status)
{
    KUint32 result = KUINT32_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KString LMI_BMC_get_active_nic(
    const CMPIBroker* cb,
    CMPIMethodMI* mi,
    const CMPIContext* context,
    const LMI_BMCRef* self,
    CMPIStatus* status)
{
    KString result = KSTRING_INIT;

    KSetStatus(status, ERR_NOT_SUPPORTED);
    return result;
}

KONKRET_REGISTRATION(
    "root/cimv2",
    "LMI_BMC",
    "LMI_BMC",
    "instance method")
