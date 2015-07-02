/*
 *  Copyright (c) 2014 Dell, Inc. All rights reserved.
 *  Licensed under the GNU Lesser General Public license, version 2.1 or any later version.
 *  Authors: Praveen K Paladugu <praveen_paladugu@dell.com>
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include <openlmi/openlmi.h>


#define DELL_LIKE_VENDORS "Dell Inc, Dell, Dell Computer Corporation"
#define IPMI_SUPPORTING_VENDORS "Dell Inc, Dell, Dell Computer Corporation"
#define BUFLEN 1024
#define WHITESPACES " \f\n\r\t\v"
#define IPv4_Add_Size 11

#define  lmi_warn(...)  printf("\nWARNING:");  printf(__VA_ARGS__)
#define  lmi_debug(...)  printf("\nDEBUG:");  printf(__VA_ARGS__)
#define  lmi_error(...)  printf("\nERROR:");  printf(__VA_ARGS__)

/*inline void * pop_calloc (size_t nmemb,size_t size,void *failed){
	void *tmp_ptr = calloc(nmemb, size);
	if (tmp_ptr == NULL)
	    goto *failed;
	return tmp_ptr;
    }
*/
typedef struct _Bmc_info{
    char **IP4Addresses;
    char **IP4Netmasks;
    char *IP4AddressSource;
    char **IP6Addresses;
    char **IP6Netmasks;
    char *IP6AddressSource;
    int vlan;
    char *PermanentMACAddress;
    char **BMC_URLs;
    char *FirmwareVersion;
    char **supportedProtos;
    char **supportedProtoVersions;
    char *active_nic;
} BMC_info;
/*
 * Run given command and store its output in buffer. Number of lines in buffer
 * is stored in buffer_size. Function skips lines starting with '#'.
 * Buffer has to be NULL, and buffer_size 0. Function will allocate necessary
 * memory, buffer can be freed with free_2d_buffer() function.
 * @param command to be run
 * @param buffer
 * @param buffer_size number of lines in buffer
 * @return negative value if problem occurred. In this case, buffer is freed.
 *          0 and positive value represent return code of command.
 */
short run_command(const char *command, char ***buffer, unsigned *buffer_size);

/*
 * Free 2D buffer.
 * @param buffer
 * @param buffer_size number of lines in buffer
 */
void free_2d_buffer(char ***buffer, unsigned *buffer_size);


/*
 * Create trimmed copy of given string. Trimmed will be any characters
 * found in delims parameter, or, if delims is NULL, any white space characters.
 * @param str
 * @param delims string containing delimiters. If NULL, white space characters
 *      are used.
 * @return trimmed string allocated with malloc or NULL if allocation failed
 *      or given string was empty or contained only delimiters
 */
char *trim(const char *str, const char *delims);

/*
 * Check if the given vendor is like Dell. Some systems are OEMed with a
 * different vendor name, even though the underlying h/w is from Dell.
 * This function checks if the vendor is among those vendors.
 * */
bool is_vendor_like_dell(char *vendor);

/*
 * From a known list, check if the vendor supports IPMI. If IPMI is suppor-
 * ted and doesn't work properly, return an ERROR.
 * If IPMI is not supported, fall back to other interfaces.
 * */
bool does_vendor_support_ipmi(char *vendor);


char * get_value_from_buffer(char *input, char **buffer,int buffer_size);

/*
 * Function populate the temporary BMC_info structure
 * */
int populate_dell_bmc_info(BMC_info *bmc_info);

/*
 * Function populate the temporary BMC_info structure
 * */
int populate_bmc_info_with_ipmi(BMC_info *bmc_info);

/*
 * Empty up all the elements of BMC Info
 * */
void free_bmc_info( BMC_info *bmc_info_ptr);

/*
 * Allocate memory and initialize the elements of bmc_info
 * */
int init_bmc_info( BMC_info *bmc_info);

/*
 * Function to return the BIOS Vendor
 * */
char *get_bios_vendor();

/*
 * Based on the vendor set the maximums to capture.
 * */
void set_bmc_max_vars();

/*
 * Alocate and initialize a new BMC_info structure
 * */
BMC_info *alloc_init_bmc_info();

/*
 * Function to free an array of pointers
 * */
void free_list(char **list, int count);
