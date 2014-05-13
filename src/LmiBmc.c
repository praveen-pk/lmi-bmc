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

/*
char * get_manufacturer ()
{
    int buf_len = BUFLEN, start_index, line_len, read;
    int status;
    char *start,*line, *vendor;
    FILE *fp;
    
    vendor = calloc(buf_len, sizeof(char) );
    lmi_debug ("Running 'dmidecode -t 1| grep Manufacturer ' command\n");
    fp = popen("sudo dmidecode -t bios | grep Vendor", "r");
    if (!fp){
	lmi_error ("Failed to run dmidecode command. Exiting.");
    }
    read = getline(&line, &line_len, fp);
    if (read == -1){
	lmi_error ("Failed while reading the output from dmidecode");
    }
    status = pclose (fp);
    if (status == -1){
	lmi_error ("Dmidecode command Failed.\n");
	printf ("%s",strerror(errno));
    }

    
    start = strchr(line, ':');
    start++;


    if (start == NULL){
	lmi_error ("Didn't find the expected delimiter in dmidecode output");
    }
    
    start_index = start - line; 

	
    while ( (line_len - start_index) > buf_len )
    {
	vendor = realloc(vendor, buf_len*2);
	buf_len *= 2;
    }
    vendor = trim (line + start_index, NULL );
    printf ("\n%s\n", vendor);
    return vendor;

}
*/

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

    /* execute command */
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


main ()
{
    char *vendor=NULL;
  
}


