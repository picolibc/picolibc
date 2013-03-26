/* tzset.c: Convert current Windows timezone to POSIX timezone information.

   Copyright 2012, 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <errno.h>
#include <stdio.h>
#include <inttypes.h>
#include <wchar.h>
#include <locale.h>
#include <getopt.h>
#include <cygwin/version.h>
#include <windows.h>

#ifndef GEOID_NOT_AVAILABLE
#define GEOID_NOT_AVAILABLE -1
#endif

/* This table maps Windows timezone keynames and countries per ISO 3166-1 to
   POSIX-compatible timezone IDs.  The information is taken from
   http://unicode.org/repos/cldr-tmp/trunk/diff/supplemental/zone_tzid.html

   The mapping from unicode.org is just a bit incomplete.  It doesn't contain
   a few timezones available on Windows XP, namely:

    Armenian Standard Time
    Mexico Standard Time
    Mexico Standard Time 2

  as well as the still (Windows 7) available

    Mid-Atlantic Standard Time

  It also doesn't contain some of the deprecated country codes used in older
  OSes, namely:

    SP (Serbian, proposed) used in XP
    CS (Serbian and Montenegro, dissolved, now RS and ME) used in Vista

  While these are apparently old, they are required here to get a complete
  mapping on all supported OSes. */
struct
{
  PCWSTR win_tzkey;
  PCWSTR country;
  PCWSTR posix_tzid;
} tzmap[] = 
{
  { L"AUS Central Standard Time", L"", L"Australia/Darwin" },
  { L"AUS Central Standard Time", L"AU", L"Australia/Darwin" },
  { L"AUS Eastern Standard Time", L"", L"Australia/Sydney" },
  { L"AUS Eastern Standard Time", L"AU", L"Australia/Sydney Australia/Melbourne" },
  { L"Afghanistan Standard Time", L"", L"Asia/Kabul" },
  { L"Afghanistan Standard Time", L"AF", L"Asia/Kabul" },
  { L"Alaskan Standard Time", L"", L"America/Anchorage" },
  { L"Alaskan Standard Time", L"US", L"America/Anchorage America/Juneau America/Nome America/Sitka America/Yakutat" },
  { L"Arab Standard Time", L"", L"Asia/Riyadh" },
  { L"Arab Standard Time", L"BH", L"Asia/Bahrain" },
  { L"Arab Standard Time", L"KW", L"Asia/Kuwait" },
  { L"Arab Standard Time", L"QA", L"Asia/Qatar" },
  { L"Arab Standard Time", L"SA", L"Asia/Riyadh" },
  { L"Arab Standard Time", L"YE", L"Asia/Aden" },
  { L"Arabian Standard Time", L"", L"Asia/Dubai" },
  { L"Arabian Standard Time", L"AE", L"Asia/Dubai" },
  { L"Arabian Standard Time", L"OM", L"Asia/Muscat" },
  { L"Arabian Standard Time", L"ZZ", L"Etc/GMT-4" },
  { L"Arabic Standard Time", L"", L"Asia/Baghdad" },
  { L"Arabic Standard Time", L"IQ", L"Asia/Baghdad" },
  { L"Argentina Standard Time", L"", L"America/Buenos_Aires" },
  { L"Argentina Standard Time", L"AR", L"America/Buenos_Aires America/Argentina/La_Rioja America/Argentina/Rio_Gallegos America/Argentina/Salta America/Argentina/San_Juan America/Argentina/San_Luis America/Argentina/Tucuman America/Argentina/Ushuaia America/Catamarca America/Cordoba America/Jujuy America/Mendoza" },
  { L"Armenian Standard Time", L"AM", L"Asia/Yerevan" },
  { L"Atlantic Standard Time", L"", L"America/Halifax" },
  { L"Atlantic Standard Time", L"BM", L"Atlantic/Bermuda" },
  { L"Atlantic Standard Time", L"CA", L"America/Halifax America/Glace_Bay America/Goose_Bay America/Moncton" },
  { L"Atlantic Standard Time", L"GL", L"America/Thule" },
  { L"Azerbaijan Standard Time", L"", L"Asia/Baku" },
  { L"Azerbaijan Standard Time", L"AZ", L"Asia/Baku" },
  { L"Azores Standard Time", L"", L"Atlantic/Azores" },
  { L"Azores Standard Time", L"GL", L"America/Scoresbysund" },
  { L"Azores Standard Time", L"PT", L"Atlantic/Azores" },
  { L"Bahia Standard Time", L"", L"America/Bahia" },
  { L"Bahia Standard Time", L"BR", L"America/Bahia" },
  { L"Bangladesh Standard Time", L"", L"Asia/Dhaka" },
  { L"Bangladesh Standard Time", L"BD", L"Asia/Dhaka" },
  { L"Bangladesh Standard Time", L"BT", L"Asia/Thimphu" },
  { L"Canada Central Standard Time", L"", L"America/Regina" },
  { L"Canada Central Standard Time", L"CA", L"America/Regina America/Swift_Current" },
  { L"Cape Verde Standard Time", L"", L"Atlantic/Cape_Verde" },
  { L"Cape Verde Standard Time", L"CV", L"Atlantic/Cape_Verde" },
  { L"Cape Verde Standard Time", L"ZZ", L"Etc/GMT+1" },
  { L"Caucasus Standard Time", L"", L"Asia/Yerevan" },
  { L"Caucasus Standard Time", L"AM", L"Asia/Yerevan" },
  { L"Cen. Australia Standard Time", L"", L"Australia/Adelaide" },
  { L"Cen. Australia Standard Time", L"AU", L"Australia/Adelaide Australia/Broken_Hill" },
  { L"Central America Standard Time", L"", L"America/Guatemala" },
  { L"Central America Standard Time", L"BZ", L"America/Belize" },
  { L"Central America Standard Time", L"CR", L"America/Costa_Rica" },
  { L"Central America Standard Time", L"EC", L"Pacific/Galapagos" },
  { L"Central America Standard Time", L"GT", L"America/Guatemala" },
  { L"Central America Standard Time", L"HN", L"America/Tegucigalpa" },
  { L"Central America Standard Time", L"NI", L"America/Managua" },
  { L"Central America Standard Time", L"SV", L"America/El_Salvador" },
  { L"Central America Standard Time", L"ZZ", L"Etc/GMT+6" },
  { L"Central Asia Standard Time", L"", L"Asia/Almaty" },
  { L"Central Asia Standard Time", L"AQ", L"Antarctica/Vostok" },
  { L"Central Asia Standard Time", L"IO", L"Indian/Chagos" },
  { L"Central Asia Standard Time", L"KG", L"Asia/Bishkek" },
  { L"Central Asia Standard Time", L"KZ", L"Asia/Almaty Asia/Qyzylorda" },
  { L"Central Asia Standard Time", L"ZZ", L"Etc/GMT-6" },
  { L"Central Brazilian Standard Time", L"", L"America/Cuiaba" },
  { L"Central Brazilian Standard Time", L"BR", L"America/Cuiaba America/Campo_Grande" },
  { L"Central Europe Standard Time", L"", L"Europe/Budapest" },
  { L"Central Europe Standard Time", L"AL", L"Europe/Tirane" },
  { L"Central Europe Standard Time", L"CS", L"Europe/Belgrade" },
  { L"Central Europe Standard Time", L"CZ", L"Europe/Prague" },
  { L"Central Europe Standard Time", L"HU", L"Europe/Budapest" },
  { L"Central Europe Standard Time", L"ME", L"Europe/Podgorica" },
  { L"Central Europe Standard Time", L"RS", L"Europe/Belgrade" },
  { L"Central Europe Standard Time", L"SI", L"Europe/Ljubljana" },
  { L"Central Europe Standard Time", L"SK", L"Europe/Bratislava" },
  { L"Central Europe Standard Time", L"SP", L"Europe/Belgrade" },
  { L"Central European Standard Time", L"", L"Europe/Warsaw" },
  { L"Central European Standard Time", L"BA", L"Europe/Sarajevo" },
  { L"Central European Standard Time", L"HR", L"Europe/Zagreb" },
  { L"Central European Standard Time", L"MK", L"Europe/Skopje" },
  { L"Central European Standard Time", L"PL", L"Europe/Warsaw" },
  { L"Central Pacific Standard Time", L"", L"Pacific/Guadalcanal" },
  { L"Central Pacific Standard Time", L"AQ", L"Antarctica/Macquarie" },
  { L"Central Pacific Standard Time", L"FM", L"Pacific/Ponape Pacific/Kosrae" },
  { L"Central Pacific Standard Time", L"NC", L"Pacific/Noumea" },
  { L"Central Pacific Standard Time", L"SB", L"Pacific/Guadalcanal" },
  { L"Central Pacific Standard Time", L"VU", L"Pacific/Efate" },
  { L"Central Pacific Standard Time", L"ZZ", L"Etc/GMT-11" },
  { L"Central Standard Time", L"", L"America/Chicago" },
  { L"Central Standard Time", L"CA", L"America/Winnipeg America/Rainy_River America/Rankin_Inlet America/Resolute" },
  { L"Central Standard Time", L"MX", L"America/Matamoros" },
  { L"Central Standard Time", L"US", L"America/Chicago America/Indiana/Knox America/Indiana/Tell_City America/Menominee America/North_Dakota/Beulah America/North_Dakota/Center America/North_Dakota/New_Salem" },
  { L"Central Standard Time", L"ZZ", L"CST6CDT" },
  { L"Central Standard Time (Mexico)", L"", L"America/Mexico_City" },
  { L"Central Standard Time (Mexico)", L"MX", L"America/Mexico_City America/Bahia_Banderas America/Cancun America/Merida America/Monterrey" },
  { L"China Standard Time", L"", L"Asia/Shanghai" },
  { L"China Standard Time", L"CN", L"Asia/Shanghai Asia/Chongqing Asia/Harbin Asia/Kashgar Asia/Urumqi" },
  { L"China Standard Time", L"HK", L"Asia/Hong_Kong" },
  { L"China Standard Time", L"MO", L"Asia/Macau" },
  { L"Dateline Standard Time", L"", L"Etc/GMT+12" },
  { L"Dateline Standard Time", L"ZZ", L"Etc/GMT+12" },
  { L"E. Africa Standard Time", L"", L"Africa/Nairobi" },
  { L"E. Africa Standard Time", L"AQ", L"Antarctica/Syowa" },
  { L"E. Africa Standard Time", L"DJ", L"Africa/Djibouti" },
  { L"E. Africa Standard Time", L"ER", L"Africa/Asmera" },
  { L"E. Africa Standard Time", L"ET", L"Africa/Addis_Ababa" },
  { L"E. Africa Standard Time", L"KE", L"Africa/Nairobi" },
  { L"E. Africa Standard Time", L"KM", L"Indian/Comoro" },
  { L"E. Africa Standard Time", L"MG", L"Indian/Antananarivo" },
  { L"E. Africa Standard Time", L"SD", L"Africa/Khartoum" },
  { L"E. Africa Standard Time", L"SO", L"Africa/Mogadishu" },
  { L"E. Africa Standard Time", L"SS", L"Africa/Juba" },
  { L"E. Africa Standard Time", L"TZ", L"Africa/Dar_es_Salaam" },
  { L"E. Africa Standard Time", L"UG", L"Africa/Kampala" },
  { L"E. Africa Standard Time", L"YT", L"Indian/Mayotte" },
  { L"E. Africa Standard Time", L"ZZ", L"Etc/GMT-3" },
  { L"E. Australia Standard Time", L"", L"Australia/Brisbane" },
  { L"E. Australia Standard Time", L"AU", L"Australia/Brisbane Australia/Lindeman" },
  { L"E. Europe Standard Time", L"", L"Asia/Nicosia" },
  { L"E. Europe Standard Time", L"CY", L"Asia/Nicosia" },
  { L"E. South America Standard Time", L"", L"America/Sao_Paulo" },
  { L"E. South America Standard Time", L"BR", L"America/Sao_Paulo" },
  { L"Eastern Standard Time", L"", L"America/New_York" },
  { L"Eastern Standard Time", L"BS", L"America/Nassau" },
  { L"Eastern Standard Time", L"CA", L"America/Toronto America/Iqaluit America/Montreal America/Nipigon America/Pangnirtung America/Thunder_Bay" },
  { L"Eastern Standard Time", L"TC", L"America/Grand_Turk" },
  { L"Eastern Standard Time", L"US", L"America/New_York America/Detroit America/Indiana/Petersburg America/Indiana/Vincennes America/Indiana/Winamac America/Kentucky/Monticello America/Louisville" },
  { L"Eastern Standard Time", L"ZZ", L"EST5EDT" },
  { L"Egypt Standard Time", L"", L"Africa/Cairo" },
  { L"Egypt Standard Time", L"EG", L"Africa/Cairo" },
  { L"Egypt Standard Time", L"PS", L"Asia/Gaza Asia/Hebron" },
  { L"Ekaterinburg Standard Time", L"", L"Asia/Yekaterinburg" },
  { L"Ekaterinburg Standard Time", L"RU", L"Asia/Yekaterinburg" },
  { L"FLE Standard Time", L"", L"Europe/Kiev" },
  { L"FLE Standard Time", L"AX", L"Europe/Mariehamn" },
  { L"FLE Standard Time", L"BG", L"Europe/Sofia" },
  { L"FLE Standard Time", L"EE", L"Europe/Tallinn" },
  { L"FLE Standard Time", L"FI", L"Europe/Helsinki" },
  { L"FLE Standard Time", L"LT", L"Europe/Vilnius" },
  { L"FLE Standard Time", L"LV", L"Europe/Riga" },
  { L"FLE Standard Time", L"UA", L"Europe/Kiev Europe/Simferopol Europe/Uzhgorod Europe/Zaporozhye" },
  { L"Fiji Standard Time", L"", L"Pacific/Fiji" },
  { L"Fiji Standard Time", L"FJ", L"Pacific/Fiji" },
  { L"GMT Standard Time", L"", L"Europe/London" },
  { L"GMT Standard Time", L"ES", L"Atlantic/Canary" },
  { L"GMT Standard Time", L"FO", L"Atlantic/Faeroe" },
  { L"GMT Standard Time", L"GB", L"Europe/London" },
  { L"GMT Standard Time", L"GG", L"Europe/Guernsey" },
  { L"GMT Standard Time", L"IE", L"Europe/Dublin" },
  { L"GMT Standard Time", L"IM", L"Europe/Isle_of_Man" },
  { L"GMT Standard Time", L"JE", L"Europe/Jersey" },
  { L"GMT Standard Time", L"PT", L"Europe/Lisbon Atlantic/Madeira" },
  { L"GTB Standard Time", L"", L"Europe/Bucharest" },
  { L"GTB Standard Time", L"GR", L"Europe/Athens" },
  { L"GTB Standard Time", L"MD", L"Europe/Chisinau" },
  { L"GTB Standard Time", L"RO", L"Europe/Bucharest" },
  { L"Georgian Standard Time", L"", L"Asia/Tbilisi" },
  { L"Georgian Standard Time", L"GE", L"Asia/Tbilisi" },
  { L"Greenland Standard Time", L"", L"America/Godthab" },
  { L"Greenland Standard Time", L"GL", L"America/Godthab" },
  { L"Greenwich Standard Time", L"", L"Atlantic/Reykjavik" },
  { L"Greenwich Standard Time", L"BF", L"Africa/Ouagadougou" },
  { L"Greenwich Standard Time", L"CI", L"Africa/Abidjan" },
  { L"Greenwich Standard Time", L"EH", L"Africa/El_Aaiun" },
  { L"Greenwich Standard Time", L"GH", L"Africa/Accra" },
  { L"Greenwich Standard Time", L"GM", L"Africa/Banjul" },
  { L"Greenwich Standard Time", L"GN", L"Africa/Conakry" },
  { L"Greenwich Standard Time", L"GW", L"Africa/Bissau" },
  { L"Greenwich Standard Time", L"IS", L"Atlantic/Reykjavik" },
  { L"Greenwich Standard Time", L"LR", L"Africa/Monrovia" },
  { L"Greenwich Standard Time", L"ML", L"Africa/Bamako" },
  { L"Greenwich Standard Time", L"MR", L"Africa/Nouakchott" },
  { L"Greenwich Standard Time", L"SH", L"Atlantic/St_Helena" },
  { L"Greenwich Standard Time", L"SL", L"Africa/Freetown" },
  { L"Greenwich Standard Time", L"SN", L"Africa/Dakar" },
  { L"Greenwich Standard Time", L"ST", L"Africa/Sao_Tome" },
  { L"Greenwich Standard Time", L"TG", L"Africa/Lome" },
  { L"Hawaiian Standard Time", L"", L"Pacific/Honolulu" },
  { L"Hawaiian Standard Time", L"CK", L"Pacific/Rarotonga" },
  { L"Hawaiian Standard Time", L"PF", L"Pacific/Tahiti" },
  { L"Hawaiian Standard Time", L"TK", L"Pacific/Fakaofo" },
  { L"Hawaiian Standard Time", L"UM", L"Pacific/Johnston" },
  { L"Hawaiian Standard Time", L"US", L"Pacific/Honolulu" },
  { L"Hawaiian Standard Time", L"ZZ", L"Etc/GMT+10" },
  { L"India Standard Time", L"", L"Asia/Calcutta" },
  { L"India Standard Time", L"IN", L"Asia/Calcutta" },
  { L"Iran Standard Time", L"", L"Asia/Tehran" },
  { L"Iran Standard Time", L"IR", L"Asia/Tehran" },
  { L"Israel Standard Time", L"", L"Asia/Jerusalem" },
  { L"Israel Standard Time", L"IL", L"Asia/Jerusalem" },
  { L"Jordan Standard Time", L"", L"Asia/Amman" },
  { L"Jordan Standard Time", L"JO", L"Asia/Amman" },
  { L"Kaliningrad Standard Time", L"", L"Europe/Kaliningrad" },
  { L"Kaliningrad Standard Time", L"BY", L"Europe/Minsk" },
  { L"Kaliningrad Standard Time", L"RU", L"Europe/Kaliningrad" },
  { L"Kamchatka Standard Time", L"", L"Asia/Kamchatka" },
  { L"Korea Standard Time", L"", L"Asia/Seoul" },
  { L"Korea Standard Time", L"KP", L"Asia/Pyongyang" },
  { L"Korea Standard Time", L"KR", L"Asia/Seoul" },
  { L"Magadan Standard Time", L"", L"Asia/Magadan" },
  { L"Magadan Standard Time", L"RU", L"Asia/Magadan Asia/Anadyr Asia/Kamchatka" },
  { L"Mauritius Standard Time", L"", L"Indian/Mauritius" },
  { L"Mauritius Standard Time", L"MU", L"Indian/Mauritius" },
  { L"Mauritius Standard Time", L"RE", L"Indian/Reunion" },
  { L"Mauritius Standard Time", L"SC", L"Indian/Mahe" },
  { L"Mexico Standard Time", L"", L"America/Mexico_City" },
  { L"Mexico Standard Time 2", L"", L"America/Mazatlan" },
  { L"Mid-Atlantic Standard Time", L"", L"Atlantic/South_Georgia" },
  { L"Middle East Standard Time", L"", L"Asia/Beirut" },
  { L"Middle East Standard Time", L"LB", L"Asia/Beirut" },
  { L"Montevideo Standard Time", L"", L"America/Montevideo" },
  { L"Montevideo Standard Time", L"UY", L"America/Montevideo" },
  { L"Morocco Standard Time", L"", L"Africa/Casablanca" },
  { L"Morocco Standard Time", L"MA", L"Africa/Casablanca" },
  { L"Mountain Standard Time", L"", L"America/Denver" },
  { L"Mountain Standard Time", L"CA", L"America/Edmonton America/Cambridge_Bay America/Inuvik America/Yellowknife" },
  { L"Mountain Standard Time", L"MX", L"America/Ojinaga" },
  { L"Mountain Standard Time", L"US", L"America/Denver America/Boise America/Shiprock" },
  { L"Mountain Standard Time", L"ZZ", L"MST7MDT" },
  { L"Mountain Standard Time (Mexico)", L"", L"America/Chihuahua" },
  { L"Mountain Standard Time (Mexico)", L"MX", L"America/Chihuahua America/Mazatlan" },
  { L"Myanmar Standard Time", L"", L"Asia/Rangoon" },
  { L"Myanmar Standard Time", L"CC", L"Indian/Cocos" },
  { L"Myanmar Standard Time", L"MM", L"Asia/Rangoon" },
  { L"N. Central Asia Standard Time", L"", L"Asia/Novosibirsk" },
  { L"N. Central Asia Standard Time", L"RU", L"Asia/Novosibirsk Asia/Novokuznetsk Asia/Omsk" },
  { L"Namibia Standard Time", L"", L"Africa/Windhoek" },
  { L"Namibia Standard Time", L"NA", L"Africa/Windhoek" },
  { L"Nepal Standard Time", L"", L"Asia/Kathmandu" },
  { L"Nepal Standard Time", L"NP", L"Asia/Kathmandu" },
  { L"New Zealand Standard Time", L"", L"Pacific/Auckland" },
  { L"New Zealand Standard Time", L"AQ", L"Antarctica/South_Pole Antarctica/McMurdo" },
  { L"New Zealand Standard Time", L"NZ", L"Pacific/Auckland" },
  { L"Newfoundland Standard Time", L"", L"America/St_Johns" },
  { L"Newfoundland Standard Time", L"CA", L"America/St_Johns" },
  { L"North Asia East Standard Time", L"", L"Asia/Irkutsk" },
  { L"North Asia East Standard Time", L"RU", L"Asia/Irkutsk" },
  { L"North Asia Standard Time", L"", L"Asia/Krasnoyarsk" },
  { L"North Asia Standard Time", L"RU", L"Asia/Krasnoyarsk" },
  { L"Pacific SA Standard Time", L"", L"America/Santiago" },
  { L"Pacific SA Standard Time", L"AQ", L"Antarctica/Palmer" },
  { L"Pacific SA Standard Time", L"CL", L"America/Santiago" },
  { L"Pacific Standard Time", L"", L"America/Los_Angeles" },
  { L"Pacific Standard Time", L"CA", L"America/Vancouver America/Dawson America/Whitehorse" },
  { L"Pacific Standard Time", L"MX", L"America/Tijuana" },
  { L"Pacific Standard Time", L"US", L"America/Los_Angeles" },
  { L"Pacific Standard Time", L"ZZ", L"PST8PDT" },
  { L"Pacific Standard Time (Mexico)", L"", L"America/Santa_Isabel" },
  { L"Pacific Standard Time (Mexico)", L"MX", L"America/Santa_Isabel" },
  { L"Pakistan Standard Time", L"", L"Asia/Karachi" },
  { L"Pakistan Standard Time", L"PK", L"Asia/Karachi" },
  { L"Paraguay Standard Time", L"", L"America/Asuncion" },
  { L"Paraguay Standard Time", L"PY", L"America/Asuncion" },
  { L"Romance Standard Time", L"", L"Europe/Paris" },
  { L"Romance Standard Time", L"BE", L"Europe/Brussels" },
  { L"Romance Standard Time", L"DK", L"Europe/Copenhagen" },
  { L"Romance Standard Time", L"ES", L"Europe/Madrid Africa/Ceuta" },
  { L"Romance Standard Time", L"FR", L"Europe/Paris" },
  { L"Russian Standard Time", L"", L"Europe/Moscow" },
  { L"Russian Standard Time", L"RU", L"Europe/Moscow Europe/Samara Europe/Volgograd" },
  { L"SA Eastern Standard Time", L"", L"America/Cayenne" },
  { L"SA Eastern Standard Time", L"AQ", L"Antarctica/Rothera" },
  { L"SA Eastern Standard Time", L"BR", L"America/Fortaleza America/Araguaina America/Belem America/Maceio America/Recife America/Santarem" },
  { L"SA Eastern Standard Time", L"GF", L"America/Cayenne" },
  { L"SA Eastern Standard Time", L"SR", L"America/Paramaribo" },
  { L"SA Eastern Standard Time", L"ZZ", L"Etc/GMT+3" },
  { L"SA Pacific Standard Time", L"", L"America/Bogota" },
  { L"SA Pacific Standard Time", L"CA", L"America/Coral_Harbour" },
  { L"SA Pacific Standard Time", L"CO", L"America/Bogota" },
  { L"SA Pacific Standard Time", L"EC", L"America/Guayaquil" },
  { L"SA Pacific Standard Time", L"HT", L"America/Port-au-Prince" },
  { L"SA Pacific Standard Time", L"JM", L"America/Jamaica" },
  { L"SA Pacific Standard Time", L"KY", L"America/Cayman" },
  { L"SA Pacific Standard Time", L"PA", L"America/Panama" },
  { L"SA Pacific Standard Time", L"PE", L"America/Lima" },
  { L"SA Pacific Standard Time", L"ZZ", L"Etc/GMT+5" },
  { L"SA Western Standard Time", L"", L"America/La_Paz" },
  { L"SA Western Standard Time", L"AG", L"America/Antigua" },
  { L"SA Western Standard Time", L"AI", L"America/Anguilla" },
  { L"SA Western Standard Time", L"AW", L"America/Aruba" },
  { L"SA Western Standard Time", L"BB", L"America/Barbados" },
  { L"SA Western Standard Time", L"BL", L"America/St_Barthelemy" },
  { L"SA Western Standard Time", L"BO", L"America/La_Paz" },
  { L"SA Western Standard Time", L"BR", L"America/Manaus America/Boa_Vista America/Eirunepe America/Porto_Velho America/Rio_Branco" },
  { L"SA Western Standard Time", L"CA", L"America/Blanc-Sablon" },
  { L"SA Western Standard Time", L"CW", L"America/Curacao" },
  { L"SA Western Standard Time", L"DM", L"America/Dominica" },
  { L"SA Western Standard Time", L"DO", L"America/Santo_Domingo" },
  { L"SA Western Standard Time", L"GD", L"America/Grenada" },
  { L"SA Western Standard Time", L"GP", L"America/Guadeloupe" },
  { L"SA Western Standard Time", L"GY", L"America/Guyana" },
  { L"SA Western Standard Time", L"KN", L"America/St_Kitts" },
  { L"SA Western Standard Time", L"LC", L"America/St_Lucia" },
  { L"SA Western Standard Time", L"MF", L"America/Marigot" },
  { L"SA Western Standard Time", L"MQ", L"America/Martinique" },
  { L"SA Western Standard Time", L"MS", L"America/Montserrat" },
  { L"SA Western Standard Time", L"PR", L"America/Puerto_Rico" },
  { L"SA Western Standard Time", L"TT", L"America/Port_of_Spain" },
  { L"SA Western Standard Time", L"VC", L"America/St_Vincent" },
  { L"SA Western Standard Time", L"VG", L"America/Tortola" },
  { L"SA Western Standard Time", L"VI", L"America/St_Thomas" },
  { L"SA Western Standard Time", L"ZZ", L"Etc/GMT+4" },
  { L"SE Asia Standard Time", L"", L"Asia/Bangkok" },
  { L"SE Asia Standard Time", L"AQ", L"Antarctica/Davis" },
  { L"SE Asia Standard Time", L"CX", L"Indian/Christmas" },
  { L"SE Asia Standard Time", L"ID", L"Asia/Jakarta Asia/Pontianak" },
  { L"SE Asia Standard Time", L"KH", L"Asia/Phnom_Penh" },
  { L"SE Asia Standard Time", L"LA", L"Asia/Vientiane" },
  { L"SE Asia Standard Time", L"MN", L"Asia/Hovd" },
  { L"SE Asia Standard Time", L"TH", L"Asia/Bangkok" },
  { L"SE Asia Standard Time", L"VN", L"Asia/Saigon" },
  { L"SE Asia Standard Time", L"ZZ", L"Etc/GMT-7" },
  { L"Samoa Standard Time", L"", L"Pacific/Apia" },
  { L"Samoa Standard Time", L"WS", L"Pacific/Apia" },
  { L"Singapore Standard Time", L"", L"Asia/Singapore" },
  { L"Singapore Standard Time", L"BN", L"Asia/Brunei" },
  { L"Singapore Standard Time", L"ID", L"Asia/Makassar" },
  { L"Singapore Standard Time", L"MY", L"Asia/Kuala_Lumpur Asia/Kuching" },
  { L"Singapore Standard Time", L"PH", L"Asia/Manila" },
  { L"Singapore Standard Time", L"SG", L"Asia/Singapore" },
  { L"Singapore Standard Time", L"ZZ", L"Etc/GMT-8" },
  { L"South Africa Standard Time", L"", L"Africa/Johannesburg" },
  { L"South Africa Standard Time", L"BI", L"Africa/Bujumbura" },
  { L"South Africa Standard Time", L"BW", L"Africa/Gaborone" },
  { L"South Africa Standard Time", L"CD", L"Africa/Lubumbashi" },
  { L"South Africa Standard Time", L"LS", L"Africa/Maseru" },
  { L"South Africa Standard Time", L"LY", L"Africa/Tripoli" },
  { L"South Africa Standard Time", L"MW", L"Africa/Blantyre" },
  { L"South Africa Standard Time", L"MZ", L"Africa/Maputo" },
  { L"South Africa Standard Time", L"RW", L"Africa/Kigali" },
  { L"South Africa Standard Time", L"SZ", L"Africa/Mbabane" },
  { L"South Africa Standard Time", L"ZA", L"Africa/Johannesburg" },
  { L"South Africa Standard Time", L"ZM", L"Africa/Lusaka" },
  { L"South Africa Standard Time", L"ZW", L"Africa/Harare" },
  { L"South Africa Standard Time", L"ZZ", L"Etc/GMT-2" },
  { L"Sri Lanka Standard Time", L"", L"Asia/Colombo" },
  { L"Sri Lanka Standard Time", L"LK", L"Asia/Colombo" },
  { L"Syria Standard Time", L"", L"Asia/Damascus" },
  { L"Syria Standard Time", L"SY", L"Asia/Damascus" },
  { L"Taipei Standard Time", L"", L"Asia/Taipei" },
  { L"Taipei Standard Time", L"TW", L"Asia/Taipei" },
  { L"Tasmania Standard Time", L"", L"Australia/Hobart" },
  { L"Tasmania Standard Time", L"AU", L"Australia/Hobart Australia/Currie" },
  { L"Tokyo Standard Time", L"", L"Asia/Tokyo" },
  { L"Tokyo Standard Time", L"ID", L"Asia/Jayapura" },
  { L"Tokyo Standard Time", L"JP", L"Asia/Tokyo" },
  { L"Tokyo Standard Time", L"PW", L"Pacific/Palau" },
  { L"Tokyo Standard Time", L"TL", L"Asia/Dili" },
  { L"Tokyo Standard Time", L"ZZ", L"Etc/GMT-9" },
  { L"Tonga Standard Time", L"", L"Pacific/Tongatapu" },
  { L"Tonga Standard Time", L"KI", L"Pacific/Enderbury" },
  { L"Tonga Standard Time", L"TO", L"Pacific/Tongatapu" },
  { L"Tonga Standard Time", L"ZZ", L"Etc/GMT-13" },
  { L"Turkey Standard Time", L"", L"Europe/Istanbul" },
  { L"Turkey Standard Time", L"TR", L"Europe/Istanbul" },
  { L"US Eastern Standard Time", L"", L"America/Indianapolis" },
  { L"US Eastern Standard Time", L"US", L"America/Indianapolis America/Indiana/Marengo America/Indiana/Vevay" },
  { L"US Mountain Standard Time", L"", L"America/Phoenix" },
  { L"US Mountain Standard Time", L"CA", L"America/Dawson_Creek" },
  { L"US Mountain Standard Time", L"MX", L"America/Hermosillo" },
  { L"US Mountain Standard Time", L"US", L"America/Phoenix" },
  { L"US Mountain Standard Time", L"ZZ", L"Etc/GMT+7" },
  { L"UTC", L"", L"Etc/GMT" },
  { L"UTC", L"GL", L"America/Danmarkshavn" },
  { L"UTC", L"ZZ", L"Etc/GMT" },
  { L"UTC+12", L"", L"Etc/GMT-12" },
  { L"UTC+12", L"KI", L"Pacific/Tarawa" },
  { L"UTC+12", L"MH", L"Pacific/Majuro Pacific/Kwajalein" },
  { L"UTC+12", L"NR", L"Pacific/Nauru" },
  { L"UTC+12", L"TV", L"Pacific/Funafuti" },
  { L"UTC+12", L"UM", L"Pacific/Wake" },
  { L"UTC+12", L"WF", L"Pacific/Wallis" },
  { L"UTC+12", L"ZZ", L"Etc/GMT-12" },
  { L"UTC-02", L"", L"Etc/GMT+2" },
  { L"UTC-02", L"BR", L"America/Noronha" },
  { L"UTC-02", L"GS", L"Atlantic/South_Georgia" },
  { L"UTC-02", L"ZZ", L"Etc/GMT+2" },
  { L"UTC-11", L"", L"Etc/GMT+11" },
  { L"UTC-11", L"AS", L"Pacific/Pago_Pago" },
  { L"UTC-11", L"NU", L"Pacific/Niue" },
  { L"UTC-11", L"UM", L"Pacific/Midway" },
  { L"UTC-11", L"ZZ", L"Etc/GMT+11" },
  { L"Ulaanbaatar Standard Time", L"", L"Asia/Ulaanbaatar" },
  { L"Ulaanbaatar Standard Time", L"MN", L"Asia/Ulaanbaatar Asia/Choibalsan" },
  { L"Venezuela Standard Time", L"", L"America/Caracas" },
  { L"Venezuela Standard Time", L"VE", L"America/Caracas" },
  { L"Vladivostok Standard Time", L"", L"Asia/Vladivostok" },
  { L"Vladivostok Standard Time", L"RU", L"Asia/Vladivostok Asia/Sakhalin" },
  { L"W. Australia Standard Time", L"", L"Australia/Perth" },
  { L"W. Australia Standard Time", L"AQ", L"Antarctica/Casey" },
  { L"W. Australia Standard Time", L"AU", L"Australia/Perth" },
  { L"W. Central Africa Standard Time", L"", L"Africa/Lagos" },
  { L"W. Central Africa Standard Time", L"AO", L"Africa/Luanda" },
  { L"W. Central Africa Standard Time", L"BJ", L"Africa/Porto-Novo" },
  { L"W. Central Africa Standard Time", L"CD", L"Africa/Kinshasa" },
  { L"W. Central Africa Standard Time", L"CF", L"Africa/Bangui" },
  { L"W. Central Africa Standard Time", L"CG", L"Africa/Brazzaville" },
  { L"W. Central Africa Standard Time", L"CM", L"Africa/Douala" },
  { L"W. Central Africa Standard Time", L"DZ", L"Africa/Algiers" },
  { L"W. Central Africa Standard Time", L"GA", L"Africa/Libreville" },
  { L"W. Central Africa Standard Time", L"GQ", L"Africa/Malabo" },
  { L"W. Central Africa Standard Time", L"NE", L"Africa/Niamey" },
  { L"W. Central Africa Standard Time", L"NG", L"Africa/Lagos" },
  { L"W. Central Africa Standard Time", L"TD", L"Africa/Ndjamena" },
  { L"W. Central Africa Standard Time", L"TN", L"Africa/Tunis" },
  { L"W. Central Africa Standard Time", L"ZZ", L"Etc/GMT-1" },
  { L"W. Europe Standard Time", L"", L"Europe/Berlin" },
  { L"W. Europe Standard Time", L"AD", L"Europe/Andorra" },
  { L"W. Europe Standard Time", L"AT", L"Europe/Vienna" },
  { L"W. Europe Standard Time", L"CH", L"Europe/Zurich" },
  { L"W. Europe Standard Time", L"DE", L"Europe/Berlin" },
  { L"W. Europe Standard Time", L"GI", L"Europe/Gibraltar" },
  { L"W. Europe Standard Time", L"IT", L"Europe/Rome" },
  { L"W. Europe Standard Time", L"LI", L"Europe/Vaduz" },
  { L"W. Europe Standard Time", L"LU", L"Europe/Luxembourg" },
  { L"W. Europe Standard Time", L"MC", L"Europe/Monaco" },
  { L"W. Europe Standard Time", L"MT", L"Europe/Malta" },
  { L"W. Europe Standard Time", L"NL", L"Europe/Amsterdam" },
  { L"W. Europe Standard Time", L"NO", L"Europe/Oslo" },
  { L"W. Europe Standard Time", L"SE", L"Europe/Stockholm" },
  { L"W. Europe Standard Time", L"SJ", L"Arctic/Longyearbyen" },
  { L"W. Europe Standard Time", L"SM", L"Europe/San_Marino" },
  { L"W. Europe Standard Time", L"VA", L"Europe/Vatican" },
  { L"West Asia Standard Time", L"", L"Asia/Tashkent" },
  { L"West Asia Standard Time", L"AQ", L"Antarctica/Mawson" },
  { L"West Asia Standard Time", L"KZ", L"Asia/Oral Asia/Aqtau Asia/Aqtobe" },
  { L"West Asia Standard Time", L"MV", L"Indian/Maldives" },
  { L"West Asia Standard Time", L"TF", L"Indian/Kerguelen" },
  { L"West Asia Standard Time", L"TJ", L"Asia/Dushanbe" },
  { L"West Asia Standard Time", L"TM", L"Asia/Ashgabat" },
  { L"West Asia Standard Time", L"UZ", L"Asia/Tashkent Asia/Samarkand" },
  { L"West Asia Standard Time", L"ZZ", L"Etc/GMT-5" },
  { L"West Pacific Standard Time", L"", L"Pacific/Port_Moresby" },
  { L"West Pacific Standard Time", L"AQ", L"Antarctica/DumontDUrville" },
  { L"West Pacific Standard Time", L"FM", L"Pacific/Truk" },
  { L"West Pacific Standard Time", L"GU", L"Pacific/Guam" },
  { L"West Pacific Standard Time", L"MP", L"Pacific/Saipan" },
  { L"West Pacific Standard Time", L"PG", L"Pacific/Port_Moresby" },
  { L"West Pacific Standard Time", L"ZZ", L"Etc/GMT-10" },
  { L"Yakutsk Standard Time", L"", L"Asia/Yakutsk" },
  { L"Yakutsk Standard Time", L"RU", L"Asia/Yakutsk" }
};

#define TZMAP_SIZE (sizeof tzmap / sizeof tzmap[0])

static struct option longopts[] =
{
  {"help", no_argument, NULL, 'h' },
  {"version", no_argument, NULL, 'V'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "hV";

#define REG_TZINFO L"SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation"
#define REG_TZDB L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"

/* Not available on pre-XP */
GEOID WINAPI (*getusergeoid)(GEOCLASS);
int WINAPI (*getgeoinfo)(GEOID, GEOTYPE, LPWSTR, int, LANGID);

static inline HKEY
reg_open (HKEY pkey, PCWSTR path, const char *msg)
{
  LONG ret;
  HKEY hkey;

  ret = RegOpenKeyExW (pkey, path, 0, KEY_READ, &hkey);
  if (ret == ERROR_SUCCESS)
    return hkey;
  if (msg)
    fprintf (stderr, "%s: cannot open registry %s, error code %" PRIu32 "\n",
	     program_invocation_short_name, msg, (unsigned int) ret);
  return NULL;
}

/* For symmetry */
#define reg_close(hkey)	RegCloseKey(hkey)

static inline BOOL
reg_query (HKEY hkey, PCWSTR value, PWCHAR buf, DWORD size, const char *msg)
{
  LONG ret;
  DWORD type;

  ret = RegQueryValueExW (hkey, value, 0, &type, (LPBYTE) buf, &size);
  if (ret == ERROR_SUCCESS)
    return TRUE;
  if (msg)
    fprintf (stderr, "%s: cannot query registry %s, error code %" PRIu32 "\n",
	     program_invocation_short_name, msg, (unsigned int) ret);
  return FALSE;
}

static inline BOOL
reg_enum (HKEY hkey, int idx, PWCHAR name, DWORD size)
{
  return RegEnumKeyExW (hkey, idx, name, &size, NULL, NULL, NULL, NULL)
	 == ERROR_SUCCESS;
}

static void
usage (FILE *stream)
{
  fprintf (stream, ""
  "Usage: %1$s [OPTION]\n"
  "\n"
  "Print POSIX-compatible timezone ID from current Windows timezone setting\n"
  "\n"
  "Options:\n"
  "  -h, --help               output usage information and exit.\n"
  "  -V, --version            output version information and exit.\n"
  "\n"
  "Use %1$s to set your TZ variable. In POSIX-compatible shells like bash,\n"
  "dash, mksh, or zsh:\n"
  "\n"
  "      export TZ=$(%1$s)\n"
  "\n"
  "In csh-compatible shells like tcsh:\n"
  "\n"
  "      setenv TZ `%1$s`\n"
  "\n", program_invocation_short_name);
};

static void
print_version ()
{
  printf ("tzset (cygwin) %d.%d.%d\n"
	  "POSIX-timezone generator\n"
	  "Copyright (C) 2012 - %s Red Hat, Inc.\n"
	  "This is free software; see the source for copying conditions.  There is NO\n"
	  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	  CYGWIN_VERSION_DLL_MAJOR / 1000,
	  CYGWIN_VERSION_DLL_MAJOR % 1000,
	  CYGWIN_VERSION_DLL_MINOR,
	  strrchr (__DATE__, ' ') + 1);
}

int
main (int argc, char **argv)
{
  BOOL ret;
  HKEY hkey, skey;
  WCHAR keyname[256], stdname[256], std2name[256], country[10], *spc;
  GEOID geo;
  int opt, idx, gotit = -1;

  setlocale (LC_ALL, "");
  while ((opt = getopt_long (argc, argv, opts, longopts, NULL)) != EOF)
    switch (opt)
      {
      case 'h':
	usage (stdout);
	return 0;
      case 'V':
	print_version ();
	return 0;
      default:
	fprintf (stderr, "Try `%s --help' for more information.\n",
		 program_invocation_short_name);
	return 1;
      }
  if (optind < argc)
    {
	usage (stderr);
	return 1;
    }

  /* First fetch current timezone information from registry. */
  hkey = reg_open (HKEY_LOCAL_MACHINE, REG_TZINFO, "timezone information");
  if (!hkey)
    return 1;
  /* Vista introduced the TimeZoneKeyName value, which simplifies the
     job a lot. */
  if (!reg_query (hkey, L"TimeZoneKeyName", keyname, sizeof keyname, NULL))
    {
      /* Pre-Vista we have a lot more to do.  First fetch the name of the
	 Standard (non-DST) timezone.  If we can't get that, give up. */
      if (!reg_query (hkey, L"StandardName", stdname, sizeof stdname,
		      "timezone information"))
	{
	  reg_close (hkey);
	  return 1;
	}
      reg_close (hkey);
      /* Now open the timezone database registry key.  Every subkey is a
         timezone.  The key name is what we're after, but to find the right
	 one, we have to compare the name of the previously fetched
	 "StandardName" with the "Std" value in the timezone info... */
      hkey = reg_open (HKEY_LOCAL_MACHINE, REG_TZDB, "timezone database");
      if (!hkey)
	return 1;
      for (idx = 0; reg_enum (hkey, idx, keyname, sizeof keyname); ++idx)
	{
	  skey = reg_open (hkey, keyname, NULL);
	  if (skey)
	    {
	      /* ...however, on MUI-enabled machines, the names are not stored
		 directly in the above StandardName, rather it is a resource
		 pointer into tzres.dll.  This is stored in MUI_Std.
		 Fortunately it's easy to recognize this situation: If
		 StandardName starts with @, it's a resource pointer, otherwise
		 it's the cleartext value. */
	      ret = reg_query (skey, stdname[0] == L'@' ? L"MUI_Std" : L"Std",
			       std2name, sizeof std2name, NULL);
	      reg_close (skey);
	      if (ret && !wcscmp (stdname, std2name))
		break;
	    }
	}
    }
  reg_close (hkey);

  /* Fetch addresses of Geo functions.  As long as we support Windows 2000
     this is required, unfortunately. */
  getusergeoid = (GEOID (WINAPI *)())
	       GetProcAddress (GetModuleHandle ("kernel32.dll"), "GetUserGeoID");
  getgeoinfo = (int (WINAPI *)(GEOID, GEOTYPE, LPWSTR, int, LANGID))
	       GetProcAddress (GetModuleHandle ("kernel32.dll"), "GetGeoInfoW");
  *country = L'\0';
  /* Post-W2K we fetch the current Geo-location of the user and convert it
     to a ISO 3166-1 compatible nation code. */
  if (getusergeoid && getgeoinfo)
    {
      geo = getusergeoid (GEOCLASS_NATION);
      if (geo != GEOID_NOT_AVAILABLE)
	getgeoinfo (geo, GEO_ISO2, country, sizeof country, 0);
    }
  /* On W2K, or if the Geo-location isn't available, we use the locale
     setting instead. */
  if (!*country)
    GetLocaleInfoW (LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME,
		    country, sizeof country);

  /* Now iterate over the mapping table and find the right entry. */
  for (idx = 0; idx < TZMAP_SIZE; ++idx)
    {
      if (!wcscmp (keyname, tzmap[idx].win_tzkey))
	{
	  if (gotit < 0)
	    gotit = idx;
	  if (!wcscmp (country, tzmap[idx].country))
	    break;
	}
      else if (gotit >= 0)
	{
	  idx = gotit;
	  break;
	}
    }
  if (idx >= TZMAP_SIZE)
    {
      if (gotit < 0)
	{
	  fprintf (stderr,
		   "%s: can't find matching POSIX timezone for "
		   "Windows timezone \"%ls\"\n",
		   program_invocation_short_name, keyname);
	  return 1;
	}
      idx = gotit;
    }
  /* Got one.  Print it.
     Note: The tzmap array is in the R/O data section on x86_64.  Don't
           try to overwrite the space, as the code did originally. */
  spc = wcschr (tzmap[idx].posix_tzid, L' ');
  if (!spc)
    spc = wcschr (tzmap[idx].posix_tzid, L'\0');
  printf ("%.*ls\n", (int) (spc - tzmap[idx].posix_tzid), tzmap[idx].posix_tzid);
  return 0;
}
