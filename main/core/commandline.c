// command.c

#include "core/commandline.h"
#include "managers/wifi_manager.h"
#include "managers/rgb_manager.h"
#include "managers/ap_manager.h"
#include "managers/ble_manager.h"
#include "managers/settings_manager.h"
#include <stdlib.h>
#include <string.h>

static Command *command_list_head = NULL;

void command_init() {
    command_list_head = NULL;
}

void register_command(const char *name, CommandFunction function) {
    // Check if the command already exists
    Command *current = command_list_head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // Command already registered
            return;
        }
        current = current->next;
    }

    // Create a new command
    Command *new_command = (Command *)malloc(sizeof(Command));
    if (new_command == NULL) {
        // Handle memory allocation failure
        return;
    }
    new_command->name = strdup(name);
    new_command->function = function;
    new_command->next = command_list_head;
    command_list_head = new_command;
}

void unregister_command(const char *name) {
    Command *current = command_list_head;
    Command *previous = NULL;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // Found the command to remove
            if (previous == NULL) {
                command_list_head = current->next;
            } else {
                previous->next = current->next;
            }
            free(current->name);
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

CommandFunction find_command(const char *name) {
    Command *current = command_list_head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current->function;
        }
        current = current->next;
    }
    return NULL;
}

void cmd_wifi_scan_start(int argc, char **argv) {
    ap_manager_add_log("WiFi scan started.\n");
    wifi_manager_start_scan();
}

void cmd_wifi_scan_stop(int argc, char **argv) {
    wifi_manager_stop_scan();
    ap_manager_add_log("WiFi scan stopped.\n");
}

void cmd_wifi_scan_results(int argc, char **argv) {
    wifi_manager_print_scan_results_with_oui();
    ap_manager_add_log("WiFi scan results displayed with OUI matching.\n");
}

void handle_list(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "-a") == 0) {
        cmd_wifi_scan_results(argc, argv);
        return;
    } 
    else if (argc > 1 && strcmp(argv[1], "-s") == 0)
    {
        wifi_manager_list_stations();
        ap_manager_add_log("Listed Stations...");
        return;
    }
    else {
        ap_manager_add_log("Usage: list -a (for Wi-Fi scan results)\n");
    }
}

void handle_beaconspam(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "-r") == 0) {
        ap_manager_add_log("Starting Random beacon spam...\n");
        wifi_manager_start_beacon(NULL);
        return;
    }

    if (argc > 1 && strcmp(argv[1], "-rr") == 0) {
        ap_manager_add_log("Starting Rickroll beacon spam...\n");
        wifi_manager_start_beacon("RICKROLL");
        return;
    }

    if (argc > 1 && strcmp(argv[1], "-l") == 0) {
        ap_manager_add_log("Starting AP List beacon spam...\n");
        wifi_manager_start_beacon("APLISTMODE");
        return;
    }

    if (argc > 1)
    {
        wifi_manager_start_beacon(argv[1]);
        return;
    }
    else {
        ap_manager_add_log("Usage: beaconspam -r (for Beacon Spam Random)\n");
    }
}


void handle_stop_spam(int argc, char **argv)
{
    wifi_manager_stop_beacon();
    ap_manager_add_log("Beacon Spam Stopped...");
}

void handle_sta_scan(int argc, char **argv)
{
    wifi_manager_start_monitor_mode(wifi_stations_sniffer_callback);
    ap_manager_add_log("Started Station Scan...");
}


void handle_attack_cmd(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        ap_manager_add_log("Deauth Attack Starting...");
        wifi_manager_start_deauth();
        return;
    }
    else 
    {
        ap_manager_add_log("Usage: attack -d (for deauthing access points)\n");
    }
}


void handle_stop_deauth(int argc, char **argv)
{
    wifi_manager_stop_deauth();
    ap_manager_add_log("Deauthing Stopped....\n");
}


void handle_select_cmd(int argc, char **argv)
{
    if (argc != 3) {
        ap_manager_add_log("Invalid number of arguments. Usage: select -a <number>\n");
        return;
    }

    if (strcmp(argv[1], "-a") == 0) {
        char *endptr;
        
        int num = (int)strtol(argv[2], &endptr, 10);


        if (*endptr == '\0') {
            wifi_manager_select_ap(num);
        } else {
            ap_manager_add_log("Error: is not a valid number.\n");
        }
    } else {
        ap_manager_add_log("Invalid option. Usage: select -a <number>\n");
    }
}

#ifdef CONFIG_BT_ENABLED

void handle_ble_scan_cmd(int argc, char**argv)
{
    if (argc > 1 && strcmp(argv[1], "-f") == 0) {
        ap_manager_add_log("Starting Find the Flippers...\n");
        ble_start_find_flippers();
        return;
    }

    if (argc > 1 && strcmp(argv[1], "-ds") == 0) {
        ap_manager_add_log("Starting BLE Spam Detector...\n");
        ble_start_blespam_detector();
        return;
    }

    if (argc > 1 && strcmp(argv[1], "-a") == 0) {
        ap_manager_add_log("Starting AirTag Scanner...\n");
        ble_start_airtag_scanner();
        return;
    }

    if (argc > 1 && strcmp(argv[1], "-r") == 0) {
        ap_manager_add_log("Scanning for Raw Packets\n");
        ble_start_raw_ble_packetscan();
        return;
    }

    if (argc > 1 && strcmp(argv[1], "-s") == 0) {
        ap_manager_add_log("Stopping BLE Scan...\n");
        ble_stop();
        return;
    }

    ap_manager_add_log("Invalid Command Syntax...");
}

#endif

void handle_set_setting(int argc, char **argv)
{
    if (argc < 3) {
        ap_manager_add_log("Error: Insufficient arguments. Expected 2 integers after the command.\n");
        return;
    }
    
    char *endptr1;
    int first_arg = strtol(argv[1], &endptr1, 10);
    
    
    if (*endptr1 != '\0') {
        ap_manager_add_log("Error: First argument is not a valid integer.\n");
        return;
    }

    
    char *endptr2;
    int second_arg = strtol(argv[2], &endptr2, 10);

    
    if (*endptr2 != '\0') {
        ap_manager_add_log("Error: Second argument is not a valid integer.\n");
        return;
    }

    int ActualSettingsIndex = first_arg;
    int ActualSettingsValue = second_arg;

    if (ActualSettingsIndex == 1) // RGB Mode
    {
        switch (ActualSettingsValue)
        {
        case 1:
        {
            settings_set_rgb_mode(&G_Settings, RGB_MODE_STEALTH);
            break;
        }
        case 2:
        {
            settings_set_rgb_mode(&G_Settings, RGB_MODE_NORMAL);
            break;
        }
        case 3:
        {
            settings_set_rgb_mode(&G_Settings, RGB_MODE_RAINBOW);
            break;
        }
        }
    }

    if (ActualSettingsIndex == 2)
    {
        switch (ActualSettingsValue)
        {
        case 1:
        {
            settings_set_channel_switch_delay(&G_Settings, 0.5);
            break;
        }
        case 2:
        {
            settings_set_channel_switch_delay(&G_Settings, 1);
            break;
        }
        case 3:
        {
            settings_set_channel_switch_delay(&G_Settings, 2);
            break;
        }
        case 4:
        {
            settings_set_channel_switch_delay(&G_Settings, 3);
            break;
        }
        case 5:
        {
            settings_set_channel_switch_delay(&G_Settings, 4);
            break;
        }
        }
    }

    if (ActualSettingsIndex == 3)
    {
        switch (ActualSettingsValue)
        {
        case 1:
        {
            settings_set_channel_hopping_enabled(&G_Settings, false);
            break;
        }
        case 2:
        {
            settings_set_channel_hopping_enabled(&G_Settings, true);
            break;
        }
        }
    }

    if (ActualSettingsIndex == 4)
    {
        switch (ActualSettingsValue)
        {
        case 1:
        {
            settings_set_random_ble_mac_enabled(&G_Settings, false);
            break;
        }
        case 2:
        {
            settings_set_random_ble_mac_enabled(&G_Settings, true);
            break;
        }
        }
    }

    settings_save(&G_Settings);
}


void handle_start_portal(int argc, char **argv)
{
    if (argc != 5) {
        printf("Error: Incorrect number of arguments.\n");
        printf("Usage: %s <URL> <SSID> <Password> <AP_ssid>\n", argv[0]);
        return;
    }

    char *url = argv[1];
    char *ssid = argv[2];
    char *password = argv[3];
    char *ap_ssid = argv[4];


    if (ssid == NULL || ssid[0] == '\0') {
        printf("Error: SSID cannot be empty.\n");
        return;
    }

    if (password == NULL || password[0] == '\0') {
        printf("Error: Password cannot be empty.\n");
        return;
    }

    if (ap_ssid == NULL || ap_ssid[0] == '\0') {
        printf("Error: AP_ssid cannot be empty.\n");
        return;
    }

     if (url == NULL || url[0] == '\0') {
        printf("Error: url cannot be empty.\n");
        return;
    }

    
    printf("Starting portal with SSID: %s, Password: %s, AP_ssid: %s\n", ssid, password, ap_ssid);

    
    wifi_manager_start_evil_portal(url, ssid, password, ap_ssid);
}


void register_commands() {
    register_command("scanap", cmd_wifi_scan_start);
    register_command("scansta", handle_sta_scan);
    register_command("stopscan", cmd_wifi_scan_stop);
    register_command("attack", handle_attack_cmd);
    register_command("list", handle_list);
    register_command("beaconspam", handle_beaconspam);
    register_command("stopspam", handle_stop_spam);
    register_command("stopdeauth", handle_stop_deauth);
    register_command("select", handle_select_cmd);
    register_command("setsetting", handle_set_setting);
    register_command("startportal", handle_start_portal);
#ifdef CONFIG_BT_ENABLED
    register_command("blescan", handle_ble_scan_cmd);
#endif
}