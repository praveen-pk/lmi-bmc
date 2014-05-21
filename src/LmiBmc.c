/*
 *  Copyright (c) 2014 Dell, Inc. All rights reserved.
 *  Licensed under the GNU General Public license, version 2.1 or any later version.
 *  Authors: Praveen K Paladugu <praveen_paladugu@dell.com>
 */
#include "LmiBmc.h"

/*From openlmi-providers/src/hardware/utils.c
 * */
char *trim(const char *str, const char *delims)
{
    char *out;
    const char *default_delims = WHITESPACES;
    size_t l;

    /* if string is empty */
    if (!str || strlen(str) < 1) {
        return NULL;
    }

    if (!delims) {
        delims = default_delims;
    }

    /* trim beginning of the string */
    while (strchr(delims, str[0]) && str[0] != '\0') {
        str++;
    }

    l = strlen(str);

    /* if string was only white spaces */
    if (l < 1) {
        return NULL;
    }

    /* shorten length of string if there are trailing white spaces */
    while (strchr(delims, str[l - 1]) && l > 0) {
        l--;
    }

    /* sanity check */
    if (l < 1) {
        return NULL;
    }

    /* copy string */
    out = strndup(str, l);
    if (!out) {
        lmi_warn("Failed to allocate memory.");
    }

    return out;
}


/*From openlmi-providers/src/hardware/utils.c
 * */
short read_fp_to_2d_buffer(FILE *fp, char ***buffer, unsigned *buffer_size)
{
    short ret = -1;
    ssize_t read;
    size_t line_len = 0;
    unsigned tmp_buffer_lines = 0, lines_read = 0;
    char **tmp_buffer = NULL, *line = NULL;

    free_2d_buffer(buffer, buffer_size);

    if (!fp) {
        lmi_warn("Given file pointer is NULL.");
        goto done;
    }

    /* allocate buffer */
    tmp_buffer_lines = 128;
    tmp_buffer = (char **)calloc(tmp_buffer_lines, sizeof(char *));
    if (!tmp_buffer) {
        lmi_warn("Failed to allocate memory.");
        tmp_buffer_lines = 0;
        goto done;
    }

    while ((read = getline(&line, &line_len, fp)) != -1) {
        /* filter comment lines */
        if (read > 0 && line[0] == '#') {
            continue;
        }

        /* reallocate if needed */
        if (lines_read >= tmp_buffer_lines) {
            tmp_buffer_lines *= 2;
            char **newtmp = (char **)realloc(tmp_buffer,
                    tmp_buffer_lines * sizeof(char *));
            if (!newtmp) {
                lmi_warn("Failed to allocate memory.");
                tmp_buffer_lines /= 2;
                goto done;
            }
            tmp_buffer = newtmp;
        }

        /* copy trimmed line to buffer */
        tmp_buffer[lines_read] = trim(line,NULL );
        if (!tmp_buffer[lines_read]) {
            tmp_buffer[lines_read] = strdup("");
            if (!tmp_buffer[lines_read]) {
                lmi_warn("Failed to allocate memory.");
                goto done;
            }
        }
        lines_read++;
    }

    if (lines_read < 1) {
        lmi_warn("No data read from given source.");
        goto done;
    }

    /* reallocate buffer to free unused space */
    if (tmp_buffer_lines > lines_read) {
        char **newtmp = (char **)realloc(tmp_buffer,
                lines_read * sizeof(char *));
        if (!newtmp) {
            lmi_warn("Failed to allocate memory.");
            goto done;
        }
        tmp_buffer = newtmp;
        tmp_buffer_lines = lines_read;
    }

    *buffer_size = tmp_buffer_lines;
    *buffer = tmp_buffer;

    ret = 0;

done:
    free(line);

    if (ret != 0) {
        free_2d_buffer(&tmp_buffer, &tmp_buffer_lines);
    }

    return ret;
}

/*From openlmi-providers/src/hardware/utils.c
 * */
void free_2d_buffer(char ***buffer, unsigned *buffer_size)
{
    unsigned i, tmp_buffer_lines = *buffer_size;
    char **tmp_buffer = *buffer;

    if (tmp_buffer && tmp_buffer_lines > 0) {
        for (i = 0; i < tmp_buffer_lines; i++) {
            free(tmp_buffer[i]);
            tmp_buffer[i] = NULL;
        }
        free(tmp_buffer);
    }

    tmp_buffer = NULL;
    *buffer_size = 0;
    *buffer = NULL;
}


/*From openlmi-providers/src/hardware/utils.c
 * */
short run_command(const char *command, char ***buffer, unsigned *buffer_size)
{
    FILE *fp = NULL;
    short ret = -1;
    char errbuf[BUFLEN];

    /* if command is empty */
    if (!command || strlen(command) < 1) {
        lmi_warn("Given command is empty.");
        goto done;
    }

    /*execute command */
    lmi_debug("Running command: \"%s\"\n", command);
    fp = popen(command, "r");
    if (!fp) {
        lmi_warn("Failed to run command: \"%s\"; Error: %s",
                command, strerror_r(errno, errbuf, sizeof(errbuf)));
        goto done;
    }

    if (read_fp_to_2d_buffer(fp, buffer, buffer_size) != 0) {
        goto done;
    }

    ret = 0;

done:
    if (fp) {
        int ret_code = pclose(fp);
        if (ret_code == -1) {
            lmi_warn("Failed to run command: \"%s\"; Error: %s",
                    command, strerror_r(errno, errbuf, sizeof(errbuf)));
            if (ret == 0) {
                ret = -1;
            }
        } else if (ret_code != 0) {
            lmi_warn("Command \"%s\" exited unexpectedly with return code: %d",
                    command, ret_code);
            ret = ret_code;
        }
    }

    if (ret < 0) {
        free_2d_buffer(buffer, buffer_size);
    }

    return ret;
}

static char * get_bios_vendor()
{
    int i;
    int buffer_size =0;
    char **buffer = NULL, *buf = NULL;
    char *vendor= NULL;

    if (run_command("sudo dmidecode -t 0| grep Vendor", &buffer, &buffer_size) != 0) {
        printf ( "Failed running the dmidecode command \n");
	return "";
    }
     
    if (buffer_size > 1) {
	lmi_error ("Unexpected output from dmidecode");
	return "";
    }
    vendor = strdup ( strchr(buffer[0],':') +1 );
    vendor = trim (vendor,"\\. ");

    return vendor;
}

bool is_vendor_like_dell(char *vendor)
{
    
    const char delim[2]=",";
    char *tmp_ven;
    char *vendor_list = strdup (DELL_LIKE_VENDORS);
    tmp_ven = strtok (vendor_list, delim );
    tmp_ven = trim (tmp_ven,NULL); 
    while ( tmp_ven != NULL)
    {
	if (strcmp (tmp_ven, vendor) == 0 ){
	    return true;
	}

	tmp_ven = strtok(NULL,delim);
	tmp_ven = trim (tmp_ven,NULL); 
	
    }
    return false;
     
}
bool command_exists (char *cmd)
{
    int ret;
    char *tmp_cmd_str = calloc (25+strlen(cmd),sizeof(char));
    if (tmp_cmd_str == NULL ){
	    lmi_error ("Failure while allocating memory in command_exists\n");
	//TODO: Handle the failure here
    }

    strcat (tmp_cmd_str, "which ");
    strcat (tmp_cmd_str,cmd);
    strcat(tmp_cmd_str," > /dev/null 2>&1");
    ret = system(tmp_cmd_str);
    free(tmp_cmd_str);	
    if (ret == 0)
    {
	return true;
    }
    else{
	return false;
    }

}

char * get_value_from_buffer(char *input, char **buffer, int buffer_size)
{
    int input_len= strlen (input);
    int i;
    char *tmp_str,*tmp_trim_str;
    
    for (i=0; i<buffer_size; i++){
	if (strncmp(input,buffer[i],input_len) == 0){

	    tmp_trim_str = trim(buffer[i]+input_len,NULL);
	    if (tmp_trim_str[0] != ':')
		continue;

	    tmp_str = strchr(buffer[i],':');
	    return trim(tmp_str+1,NULL);
	}
    }

    return NULL;
}



int populate_dell_bmc_info(BMC_info *bmc_info)
{
    int buffer_size=0, tmp_len=0;
    char **buffer=NULL;
    char *tmp_str;
    int ret=0;

    if ( ! command_exists ("ipmitool ")){
	lmi_error ("ipmitool comman doesn't exist. send empty instance");
	//TODO: Handle Failure.
    }
   // if (run_command("ipmitool lan print 1", &buffer, &buffer_size) != 0) {
    if (run_command("cat /root/ipmi_output", &buffer, &buffer_size) != 0) {
        printf ("Failed running the ipmitool command. Check if ipmi service is running \n");
	goto failed;
    }

   lmi_debug ("After running ipmitool lan print  command\n"); 
    /*
 *  Populate only one IPV4 Address
 *  */
    tmp_str = get_value_from_buffer("IP Address", buffer,buffer_size);
    bmc_info->IP4Addresses = (char **) calloc(1, sizeof(char*));
    bmc_info->IP4Addresses[0] =tmp_str;

/*Populate Netmask*/
    tmp_str = get_value_from_buffer("Subnet Mask", buffer,buffer_size);
    bmc_info->IP4Netmasks = (char**)calloc(1, sizeof(char*));
    if (bmc_info->IP4Netmasks == NULL)
	goto failed;
    bmc_info->IP4Netmasks[0] =tmp_str;
/*Populate IP Source*/   
    tmp_str = get_value_from_buffer("IP Address Source", buffer,buffer_size);
    bmc_info->IP4AddressSource = (char*)calloc(1, sizeof(char*));
    if (bmc_info->IP4AddressSource == NULL)
	goto failed;
    bmc_info->IP4AddressSource =tmp_str;

    /*IPv6 address not supported yet*/
    bmc_info->IP6Addresses = NULL;
    bmc_info->IP6AddressSource = NULL;
    bmc_info->IP6Netmasks = NULL;

/*MAC Address */
    tmp_str = get_value_from_buffer("MAC Address", buffer,buffer_size);
    bmc_info->PermanentMACAddress = (char *)calloc(1, sizeof(char*));
    if (bmc_info->PermanentMACAddress == NULL)
	goto failed; 
    bmc_info->PermanentMACAddress = tmp_str;
    
/* Only have one URL */
    bmc_info->BMC_URLs = (char **)calloc( 1 ,sizeof(char*));
    tmp_str = (char *) calloc(10+strlen(bmc_info->IP4Addresses[0]), sizeof(char));
    strcpy(tmp_str,"https://");
    strcat (tmp_str,bmc_info->IP4Addresses[0]);
    bmc_info->BMC_URLs[0] = tmp_str ;
    
    if ( strcmp( get_value_from_buffer("802.1q VLAN ID", buffer,buffer_size), "Disabled"  ) == 0 )
    {
	bmc_info->vlan = 0;
    }

    lmi_debug("Before running ipmitool mc info\n\n\n");

//    if (run_command("ipmitool mc info", &buffer, &buffer_size) != 0) {
    if (run_command("cat /root/ipmi_mc_output", &buffer, &buffer_size) != 0) {
        printf ( "Failed running the ipmitool command. Check if ipmi service is running \n");
        return 1;
    }

/*IPMI Version*/
    tmp_str = get_value_from_buffer("IPMI Version", buffer,buffer_size);
    bmc_info->supportedProtoVersions = (char**)calloc(1, sizeof(char*));
    bmc_info->supportedProtoVersions[0] =tmp_str;
   
    tmp_str = calloc(4, sizeof(char));
    strcpy(tmp_str,"IPMI");
    bmc_info->supportedProtos = (char**)calloc(1, sizeof(char*));
    bmc_info->supportedProtos[0] =tmp_str;
    
    lmi_debug("End of Populating\n");

    return 0;

failed:
    printf ("One of the allocations failed. Returning NULL");
    free_bmc_info(bmc_info);
    return 1;

}

void free_bmc_info( BMC_info *bmc_info)
{

int i = 0;

if (bmc_info->IP4Addresses != NULL)
{
    int i=0;
    while (bmc_info->IP4Addresses[i] != NULL)
    {
	free(bmc_info->IP4Addresses[i]);
	i++;
    }
    free(bmc_info->IP4Addresses);
}


if (bmc_info->IP4Netmasks !=NULL )
{
    i=0;

    while (bmc_info->IP4Netmasks[i] != NULL)
    {
	free(bmc_info->IP4Netmasks[i]);
	i++;
    }
    free(bmc_info->IP4Netmasks);
}


free(bmc_info->IP4AddressSource);

free(bmc_info->PermanentMACAddress);
//TODO: Handle the remaining elements too. 

free (bmc_info);
}

int init_bmc_info( BMC_info *bmc_info)
{

    bmc_info = calloc(1, sizeof(BMC_info));
    if (bmc_info == NULL)
    {
	lmi_error("return empty\n");
	return 1;
    }
    
    bmc_info->IP4Addresses=NULL;
    bmc_info->IP4Netmasks=NULL;
    bmc_info->IP4Netmasks=NULL;
    bmc_info->IP4AddressSource=NULL;
    bmc_info->PermanentMACAddress=NULL;
}

/*main ()
{
    char *vendor=NULL;
    BMC_info *bmc_info;
    init_bmc_info (bmc_info);

    populate_dell_bmc_info (bmc_info);
    printf ("IP: %s\n",bmc_info->IP4Addresses[0]);
    printf ("IP: %s\n",bmc_info->IP4AddressSource);
    printf ("MAC: %s\n",bmc_info->PermanentMACAddress);

  
}*/


