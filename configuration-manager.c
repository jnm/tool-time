/* jnm 20140809 */

/* Make sure to use absolute paths only! */
const char configtest[] = "/usr/sbin/apache2ctl configtest";
const char graceful[] = "/usr/sbin/apache2ctl graceful";
const char maintenance_conf[] = "/opt/maintenance/maintenance.conf";
const char normal_conf[] = "/opt/maintenance/normal.conf";
const char active_conf[] = "/opt/maintenance/active.conf";
const char address_marker[] = "{address}";

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

int main(int argc, char* argv[])
{
    const char* source_path;
    char* address = NULL;

    if(argc == 2 && strcmp(argv[1], "normal") == 0)
        source_path = normal_conf;
    else if(argc == 3 && strcmp(argv[1], "maintenance") == 0)
    {
        regex_t regex;
        address = argv[2];
        regcomp(&regex, "^\\([0-9]\\{1,3\\}\\.\\)\\{3\\}[0-9]\\{1,3\\}$", 0);
        if(regexec(&regex, address, 0, NULL, 0) == REG_NOMATCH)
        {
            fprintf(stderr, "Invalid address.\n");
            return 1;
        }
        source_path = maintenance_conf;
    }
    else
    {
        fprintf(stderr, "Usage: %s maintenance|normal [address]\n", argv[0]);
        return 1;
    }

    FILE* source = fopen(source_path, "r");
    if(source == NULL)
    {
        fprintf(stderr, "Failed to open source configuration file.\n");
        return 1;
    }
    FILE* destination = fopen(active_conf, "w");
    if(destination == NULL)
    {
        fprintf(stderr, "Failed to open destination configuration file.\n");
        fclose(source);
        return 1;
    }
    char* line = NULL;
    size_t line_size = 0;
    ssize_t read_length;
    char* marker_location = NULL;
    while((read_length = getline(&line, &line_size, source)) != -1)
    {
        if(address != NULL && (marker_location = strstr(line, address_marker)) != NULL)
        {
            /* Write the part of the line preceeding the marker */
            fwrite(line, sizeof(char), marker_location - line, destination);
            /* Write the address */
            fprintf(destination, "%s", address);
            /* Write the part of the line following the marker */
            fprintf(destination, "%s", marker_location + strlen(address_marker)); 
        }
        else
            fwrite(line, sizeof(char), read_length, destination);
    }
    if(line)
        free(line);
    fclose(source);
    fclose(destination);
    if(system(configtest) == 0)
        return system(graceful);
    return 1;
}
