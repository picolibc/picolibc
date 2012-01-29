/* tzset.c: Convert current Windows timezone to POSIX timezone information.

   Copyright 2012 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <errno.h>
#include <stdio.h>
#include <string.h>
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
  const char *win_tzkey;
  const char *country;
  const char *posix_tzid;
} tzmap[] = 
{
  { "AUS Central Standard Time", "", "Australia/Darwin" },
  { "AUS Central Standard Time", "AU", "Australia/Darwin" },
  { "AUS Eastern Standard Time", "", "Australia/Sydney" },
  { "AUS Eastern Standard Time", "AU", "Australia/Sydney Australia/Melbourne" },
  { "Afghanistan Standard Time", "", "Asia/Kabul" },
  { "Afghanistan Standard Time", "AF", "Asia/Kabul" },
  { "Alaskan Standard Time", "", "America/Anchorage" },
  { "Alaskan Standard Time", "US", "America/Anchorage America/Juneau America/Nome America/Sitka America/Yakutat" },
  { "Arab Standard Time", "", "Asia/Riyadh" },
  { "Arab Standard Time", "BH", "Asia/Bahrain" },
  { "Arab Standard Time", "KW", "Asia/Kuwait" },
  { "Arab Standard Time", "QA", "Asia/Qatar" },
  { "Arab Standard Time", "SA", "Asia/Riyadh" },
  { "Arab Standard Time", "YE", "Asia/Aden" },
  { "Arabian Standard Time", "", "Asia/Dubai" },
  { "Arabian Standard Time", "AE", "Asia/Dubai" },
  { "Arabian Standard Time", "OM", "Asia/Muscat" },
  { "Arabian Standard Time", "ZZ", "Etc/GMT-4" },
  { "Arabic Standard Time", "", "Asia/Baghdad" },
  { "Arabic Standard Time", "IQ", "Asia/Baghdad" },
  { "Argentina Standard Time", "", "America/Buenos_Aires" },
  { "Argentina Standard Time", "AR", "America/Buenos_Aires America/Argentina/La_Rioja America/Argentina/Rio_Gallegos America/Argentina/Salta America/Argentina/San_Juan America/Argentina/San_Luis America/Argentina/Tucuman America/Argentina/Ushuaia America/Catamarca America/Cordoba America/Jujuy America/Mendoza" },
  { "Armenian Standard Time", "AM", "Asia/Yerevan" },
  { "Atlantic Standard Time", "", "America/Halifax" },
  { "Atlantic Standard Time", "BM", "Atlantic/Bermuda" },
  { "Atlantic Standard Time", "CA", "America/Halifax America/Glace_Bay America/Goose_Bay America/Moncton" },
  { "Atlantic Standard Time", "GL", "America/Thule" },
  { "Azerbaijan Standard Time", "", "Asia/Baku" },
  { "Azerbaijan Standard Time", "AZ", "Asia/Baku" },
  { "Azores Standard Time", "", "Atlantic/Azores" },
  { "Azores Standard Time", "GL", "America/Scoresbysund" },
  { "Azores Standard Time", "PT", "Atlantic/Azores" },
  { "Bahia Standard Time", "", "America/Bahia" },
  { "Bahia Standard Time", "BR", "America/Bahia" },
  { "Bangladesh Standard Time", "", "Asia/Dhaka" },
  { "Bangladesh Standard Time", "BD", "Asia/Dhaka" },
  { "Bangladesh Standard Time", "BT", "Asia/Thimphu" },
  { "Canada Central Standard Time", "", "America/Regina" },
  { "Canada Central Standard Time", "CA", "America/Regina America/Swift_Current" },
  { "Cape Verde Standard Time", "", "Atlantic/Cape_Verde" },
  { "Cape Verde Standard Time", "CV", "Atlantic/Cape_Verde" },
  { "Cape Verde Standard Time", "ZZ", "Etc/GMT+1" },
  { "Caucasus Standard Time", "", "Asia/Yerevan" },
  { "Caucasus Standard Time", "AM", "Asia/Yerevan" },
  { "Cen. Australia Standard Time", "", "Australia/Adelaide" },
  { "Cen. Australia Standard Time", "AU", "Australia/Adelaide Australia/Broken_Hill" },
  { "Central America Standard Time", "", "America/Guatemala" },
  { "Central America Standard Time", "BZ", "America/Belize" },
  { "Central America Standard Time", "CR", "America/Costa_Rica" },
  { "Central America Standard Time", "EC", "Pacific/Galapagos" },
  { "Central America Standard Time", "GT", "America/Guatemala" },
  { "Central America Standard Time", "HN", "America/Tegucigalpa" },
  { "Central America Standard Time", "NI", "America/Managua" },
  { "Central America Standard Time", "SV", "America/El_Salvador" },
  { "Central America Standard Time", "ZZ", "Etc/GMT+6" },
  { "Central Asia Standard Time", "", "Asia/Almaty" },
  { "Central Asia Standard Time", "AQ", "Antarctica/Vostok" },
  { "Central Asia Standard Time", "IO", "Indian/Chagos" },
  { "Central Asia Standard Time", "KG", "Asia/Bishkek" },
  { "Central Asia Standard Time", "KZ", "Asia/Almaty Asia/Qyzylorda" },
  { "Central Asia Standard Time", "ZZ", "Etc/GMT-6" },
  { "Central Brazilian Standard Time", "", "America/Cuiaba" },
  { "Central Brazilian Standard Time", "BR", "America/Cuiaba America/Campo_Grande" },
  { "Central Europe Standard Time", "", "Europe/Budapest" },
  { "Central Europe Standard Time", "AL", "Europe/Tirane" },
  { "Central Europe Standard Time", "CS", "Europe/Belgrade" },
  { "Central Europe Standard Time", "CZ", "Europe/Prague" },
  { "Central Europe Standard Time", "HU", "Europe/Budapest" },
  { "Central Europe Standard Time", "ME", "Europe/Podgorica" },
  { "Central Europe Standard Time", "RS", "Europe/Belgrade" },
  { "Central Europe Standard Time", "SI", "Europe/Ljubljana" },
  { "Central Europe Standard Time", "SK", "Europe/Bratislava" },
  { "Central Europe Standard Time", "SP", "Europe/Belgrade" },
  { "Central European Standard Time", "", "Europe/Warsaw" },
  { "Central European Standard Time", "BA", "Europe/Sarajevo" },
  { "Central European Standard Time", "HR", "Europe/Zagreb" },
  { "Central European Standard Time", "MK", "Europe/Skopje" },
  { "Central European Standard Time", "PL", "Europe/Warsaw" },
  { "Central Pacific Standard Time", "", "Pacific/Guadalcanal" },
  { "Central Pacific Standard Time", "AQ", "Antarctica/Macquarie" },
  { "Central Pacific Standard Time", "FM", "Pacific/Ponape Pacific/Kosrae" },
  { "Central Pacific Standard Time", "NC", "Pacific/Noumea" },
  { "Central Pacific Standard Time", "SB", "Pacific/Guadalcanal" },
  { "Central Pacific Standard Time", "VU", "Pacific/Efate" },
  { "Central Pacific Standard Time", "ZZ", "Etc/GMT-11" },
  { "Central Standard Time", "", "America/Chicago" },
  { "Central Standard Time", "CA", "America/Winnipeg America/Rainy_River America/Rankin_Inlet America/Resolute" },
  { "Central Standard Time", "MX", "America/Matamoros" },
  { "Central Standard Time", "US", "America/Chicago America/Indiana/Knox America/Indiana/Tell_City America/Menominee America/North_Dakota/Beulah America/North_Dakota/Center America/North_Dakota/New_Salem" },
  { "Central Standard Time", "ZZ", "CST6CDT" },
  { "Central Standard Time (Mexico)", "", "America/Mexico_City" },
  { "Central Standard Time (Mexico)", "MX", "America/Mexico_City America/Bahia_Banderas America/Cancun America/Merida America/Monterrey" },
  { "China Standard Time", "", "Asia/Shanghai" },
  { "China Standard Time", "CN", "Asia/Shanghai Asia/Chongqing Asia/Harbin Asia/Kashgar Asia/Urumqi" },
  { "China Standard Time", "HK", "Asia/Hong_Kong" },
  { "China Standard Time", "MO", "Asia/Macau" },
  { "Dateline Standard Time", "", "Etc/GMT+12" },
  { "Dateline Standard Time", "ZZ", "Etc/GMT+12" },
  { "E. Africa Standard Time", "", "Africa/Nairobi" },
  { "E. Africa Standard Time", "AQ", "Antarctica/Syowa" },
  { "E. Africa Standard Time", "DJ", "Africa/Djibouti" },
  { "E. Africa Standard Time", "ER", "Africa/Asmera" },
  { "E. Africa Standard Time", "ET", "Africa/Addis_Ababa" },
  { "E. Africa Standard Time", "KE", "Africa/Nairobi" },
  { "E. Africa Standard Time", "KM", "Indian/Comoro" },
  { "E. Africa Standard Time", "MG", "Indian/Antananarivo" },
  { "E. Africa Standard Time", "SD", "Africa/Khartoum" },
  { "E. Africa Standard Time", "SO", "Africa/Mogadishu" },
  { "E. Africa Standard Time", "SS", "Africa/Juba" },
  { "E. Africa Standard Time", "TZ", "Africa/Dar_es_Salaam" },
  { "E. Africa Standard Time", "UG", "Africa/Kampala" },
  { "E. Africa Standard Time", "YT", "Indian/Mayotte" },
  { "E. Africa Standard Time", "ZZ", "Etc/GMT-3" },
  { "E. Australia Standard Time", "", "Australia/Brisbane" },
  { "E. Australia Standard Time", "AU", "Australia/Brisbane Australia/Lindeman" },
  { "E. Europe Standard Time", "", "Asia/Nicosia" },
  { "E. Europe Standard Time", "CY", "Asia/Nicosia" },
  { "E. South America Standard Time", "", "America/Sao_Paulo" },
  { "E. South America Standard Time", "BR", "America/Sao_Paulo" },
  { "Eastern Standard Time", "", "America/New_York" },
  { "Eastern Standard Time", "BS", "America/Nassau" },
  { "Eastern Standard Time", "CA", "America/Toronto America/Iqaluit America/Montreal America/Nipigon America/Pangnirtung America/Thunder_Bay" },
  { "Eastern Standard Time", "TC", "America/Grand_Turk" },
  { "Eastern Standard Time", "US", "America/New_York America/Detroit America/Indiana/Petersburg America/Indiana/Vincennes America/Indiana/Winamac America/Kentucky/Monticello America/Louisville" },
  { "Eastern Standard Time", "ZZ", "EST5EDT" },
  { "Egypt Standard Time", "", "Africa/Cairo" },
  { "Egypt Standard Time", "EG", "Africa/Cairo" },
  { "Egypt Standard Time", "PS", "Asia/Gaza Asia/Hebron" },
  { "Ekaterinburg Standard Time", "", "Asia/Yekaterinburg" },
  { "Ekaterinburg Standard Time", "RU", "Asia/Yekaterinburg" },
  { "FLE Standard Time", "", "Europe/Kiev" },
  { "FLE Standard Time", "AX", "Europe/Mariehamn" },
  { "FLE Standard Time", "BG", "Europe/Sofia" },
  { "FLE Standard Time", "EE", "Europe/Tallinn" },
  { "FLE Standard Time", "FI", "Europe/Helsinki" },
  { "FLE Standard Time", "LT", "Europe/Vilnius" },
  { "FLE Standard Time", "LV", "Europe/Riga" },
  { "FLE Standard Time", "UA", "Europe/Kiev Europe/Simferopol Europe/Uzhgorod Europe/Zaporozhye" },
  { "Fiji Standard Time", "", "Pacific/Fiji" },
  { "Fiji Standard Time", "FJ", "Pacific/Fiji" },
  { "GMT Standard Time", "", "Europe/London" },
  { "GMT Standard Time", "ES", "Atlantic/Canary" },
  { "GMT Standard Time", "FO", "Atlantic/Faeroe" },
  { "GMT Standard Time", "GB", "Europe/London" },
  { "GMT Standard Time", "GG", "Europe/Guernsey" },
  { "GMT Standard Time", "IE", "Europe/Dublin" },
  { "GMT Standard Time", "IM", "Europe/Isle_of_Man" },
  { "GMT Standard Time", "JE", "Europe/Jersey" },
  { "GMT Standard Time", "PT", "Europe/Lisbon Atlantic/Madeira" },
  { "GTB Standard Time", "", "Europe/Bucharest" },
  { "GTB Standard Time", "GR", "Europe/Athens" },
  { "GTB Standard Time", "MD", "Europe/Chisinau" },
  { "GTB Standard Time", "RO", "Europe/Bucharest" },
  { "Georgian Standard Time", "", "Asia/Tbilisi" },
  { "Georgian Standard Time", "GE", "Asia/Tbilisi" },
  { "Greenland Standard Time", "", "America/Godthab" },
  { "Greenland Standard Time", "GL", "America/Godthab" },
  { "Greenwich Standard Time", "", "Atlantic/Reykjavik" },
  { "Greenwich Standard Time", "BF", "Africa/Ouagadougou" },
  { "Greenwich Standard Time", "CI", "Africa/Abidjan" },
  { "Greenwich Standard Time", "EH", "Africa/El_Aaiun" },
  { "Greenwich Standard Time", "GH", "Africa/Accra" },
  { "Greenwich Standard Time", "GM", "Africa/Banjul" },
  { "Greenwich Standard Time", "GN", "Africa/Conakry" },
  { "Greenwich Standard Time", "GW", "Africa/Bissau" },
  { "Greenwich Standard Time", "IS", "Atlantic/Reykjavik" },
  { "Greenwich Standard Time", "LR", "Africa/Monrovia" },
  { "Greenwich Standard Time", "ML", "Africa/Bamako" },
  { "Greenwich Standard Time", "MR", "Africa/Nouakchott" },
  { "Greenwich Standard Time", "SH", "Atlantic/St_Helena" },
  { "Greenwich Standard Time", "SL", "Africa/Freetown" },
  { "Greenwich Standard Time", "SN", "Africa/Dakar" },
  { "Greenwich Standard Time", "ST", "Africa/Sao_Tome" },
  { "Greenwich Standard Time", "TG", "Africa/Lome" },
  { "Hawaiian Standard Time", "", "Pacific/Honolulu" },
  { "Hawaiian Standard Time", "CK", "Pacific/Rarotonga" },
  { "Hawaiian Standard Time", "PF", "Pacific/Tahiti" },
  { "Hawaiian Standard Time", "TK", "Pacific/Fakaofo" },
  { "Hawaiian Standard Time", "UM", "Pacific/Johnston" },
  { "Hawaiian Standard Time", "US", "Pacific/Honolulu" },
  { "Hawaiian Standard Time", "ZZ", "Etc/GMT+10" },
  { "India Standard Time", "", "Asia/Calcutta" },
  { "India Standard Time", "IN", "Asia/Calcutta" },
  { "Iran Standard Time", "", "Asia/Tehran" },
  { "Iran Standard Time", "IR", "Asia/Tehran" },
  { "Israel Standard Time", "", "Asia/Jerusalem" },
  { "Israel Standard Time", "IL", "Asia/Jerusalem" },
  { "Jordan Standard Time", "", "Asia/Amman" },
  { "Jordan Standard Time", "JO", "Asia/Amman" },
  { "Kaliningrad Standard Time", "", "Europe/Kaliningrad" },
  { "Kaliningrad Standard Time", "BY", "Europe/Minsk" },
  { "Kaliningrad Standard Time", "RU", "Europe/Kaliningrad" },
  { "Kamchatka Standard Time", "", "Asia/Kamchatka" },
  { "Korea Standard Time", "", "Asia/Seoul" },
  { "Korea Standard Time", "KP", "Asia/Pyongyang" },
  { "Korea Standard Time", "KR", "Asia/Seoul" },
  { "Magadan Standard Time", "", "Asia/Magadan" },
  { "Magadan Standard Time", "RU", "Asia/Magadan Asia/Anadyr Asia/Kamchatka" },
  { "Mauritius Standard Time", "", "Indian/Mauritius" },
  { "Mauritius Standard Time", "MU", "Indian/Mauritius" },
  { "Mauritius Standard Time", "RE", "Indian/Reunion" },
  { "Mauritius Standard Time", "SC", "Indian/Mahe" },
  { "Mexico Standard Time", "", "America/Mexico_City" },
  { "Mexico Standard Time 2", "", "America/Mazatlan" },
  { "Mid-Atlantic Standard Time", "", "Atlantic/South_Georgia" },
  { "Middle East Standard Time", "", "Asia/Beirut" },
  { "Middle East Standard Time", "LB", "Asia/Beirut" },
  { "Montevideo Standard Time", "", "America/Montevideo" },
  { "Montevideo Standard Time", "UY", "America/Montevideo" },
  { "Morocco Standard Time", "", "Africa/Casablanca" },
  { "Morocco Standard Time", "MA", "Africa/Casablanca" },
  { "Mountain Standard Time", "", "America/Denver" },
  { "Mountain Standard Time", "CA", "America/Edmonton America/Cambridge_Bay America/Inuvik America/Yellowknife" },
  { "Mountain Standard Time", "MX", "America/Ojinaga" },
  { "Mountain Standard Time", "US", "America/Denver America/Boise America/Shiprock" },
  { "Mountain Standard Time", "ZZ", "MST7MDT" },
  { "Mountain Standard Time (Mexico)", "", "America/Chihuahua" },
  { "Mountain Standard Time (Mexico)", "MX", "America/Chihuahua America/Mazatlan" },
  { "Myanmar Standard Time", "", "Asia/Rangoon" },
  { "Myanmar Standard Time", "CC", "Indian/Cocos" },
  { "Myanmar Standard Time", "MM", "Asia/Rangoon" },
  { "N. Central Asia Standard Time", "", "Asia/Novosibirsk" },
  { "N. Central Asia Standard Time", "RU", "Asia/Novosibirsk Asia/Novokuznetsk Asia/Omsk" },
  { "Namibia Standard Time", "", "Africa/Windhoek" },
  { "Namibia Standard Time", "NA", "Africa/Windhoek" },
  { "Nepal Standard Time", "", "Asia/Kathmandu" },
  { "Nepal Standard Time", "NP", "Asia/Kathmandu" },
  { "New Zealand Standard Time", "", "Pacific/Auckland" },
  { "New Zealand Standard Time", "AQ", "Antarctica/South_Pole Antarctica/McMurdo" },
  { "New Zealand Standard Time", "NZ", "Pacific/Auckland" },
  { "Newfoundland Standard Time", "", "America/St_Johns" },
  { "Newfoundland Standard Time", "CA", "America/St_Johns" },
  { "North Asia East Standard Time", "", "Asia/Irkutsk" },
  { "North Asia East Standard Time", "RU", "Asia/Irkutsk" },
  { "North Asia Standard Time", "", "Asia/Krasnoyarsk" },
  { "North Asia Standard Time", "RU", "Asia/Krasnoyarsk" },
  { "Pacific SA Standard Time", "", "America/Santiago" },
  { "Pacific SA Standard Time", "AQ", "Antarctica/Palmer" },
  { "Pacific SA Standard Time", "CL", "America/Santiago" },
  { "Pacific Standard Time", "", "America/Los_Angeles" },
  { "Pacific Standard Time", "CA", "America/Vancouver America/Dawson America/Whitehorse" },
  { "Pacific Standard Time", "MX", "America/Tijuana" },
  { "Pacific Standard Time", "US", "America/Los_Angeles" },
  { "Pacific Standard Time", "ZZ", "PST8PDT" },
  { "Pacific Standard Time (Mexico)", "", "America/Santa_Isabel" },
  { "Pacific Standard Time (Mexico)", "MX", "America/Santa_Isabel" },
  { "Pakistan Standard Time", "", "Asia/Karachi" },
  { "Pakistan Standard Time", "PK", "Asia/Karachi" },
  { "Paraguay Standard Time", "", "America/Asuncion" },
  { "Paraguay Standard Time", "PY", "America/Asuncion" },
  { "Romance Standard Time", "", "Europe/Paris" },
  { "Romance Standard Time", "BE", "Europe/Brussels" },
  { "Romance Standard Time", "DK", "Europe/Copenhagen" },
  { "Romance Standard Time", "ES", "Europe/Madrid Africa/Ceuta" },
  { "Romance Standard Time", "FR", "Europe/Paris" },
  { "Russian Standard Time", "", "Europe/Moscow" },
  { "Russian Standard Time", "RU", "Europe/Moscow Europe/Samara Europe/Volgograd" },
  { "SA Eastern Standard Time", "", "America/Cayenne" },
  { "SA Eastern Standard Time", "AQ", "Antarctica/Rothera" },
  { "SA Eastern Standard Time", "BR", "America/Fortaleza America/Araguaina America/Belem America/Maceio America/Recife America/Santarem" },
  { "SA Eastern Standard Time", "GF", "America/Cayenne" },
  { "SA Eastern Standard Time", "SR", "America/Paramaribo" },
  { "SA Eastern Standard Time", "ZZ", "Etc/GMT+3" },
  { "SA Pacific Standard Time", "", "America/Bogota" },
  { "SA Pacific Standard Time", "CA", "America/Coral_Harbour" },
  { "SA Pacific Standard Time", "CO", "America/Bogota" },
  { "SA Pacific Standard Time", "EC", "America/Guayaquil" },
  { "SA Pacific Standard Time", "HT", "America/Port-au-Prince" },
  { "SA Pacific Standard Time", "JM", "America/Jamaica" },
  { "SA Pacific Standard Time", "KY", "America/Cayman" },
  { "SA Pacific Standard Time", "PA", "America/Panama" },
  { "SA Pacific Standard Time", "PE", "America/Lima" },
  { "SA Pacific Standard Time", "ZZ", "Etc/GMT+5" },
  { "SA Western Standard Time", "", "America/La_Paz" },
  { "SA Western Standard Time", "AG", "America/Antigua" },
  { "SA Western Standard Time", "AI", "America/Anguilla" },
  { "SA Western Standard Time", "AW", "America/Aruba" },
  { "SA Western Standard Time", "BB", "America/Barbados" },
  { "SA Western Standard Time", "BL", "America/St_Barthelemy" },
  { "SA Western Standard Time", "BO", "America/La_Paz" },
  { "SA Western Standard Time", "BR", "America/Manaus America/Boa_Vista America/Eirunepe America/Porto_Velho America/Rio_Branco" },
  { "SA Western Standard Time", "CA", "America/Blanc-Sablon" },
  { "SA Western Standard Time", "CW", "America/Curacao" },
  { "SA Western Standard Time", "DM", "America/Dominica" },
  { "SA Western Standard Time", "DO", "America/Santo_Domingo" },
  { "SA Western Standard Time", "GD", "America/Grenada" },
  { "SA Western Standard Time", "GP", "America/Guadeloupe" },
  { "SA Western Standard Time", "GY", "America/Guyana" },
  { "SA Western Standard Time", "KN", "America/St_Kitts" },
  { "SA Western Standard Time", "LC", "America/St_Lucia" },
  { "SA Western Standard Time", "MF", "America/Marigot" },
  { "SA Western Standard Time", "MQ", "America/Martinique" },
  { "SA Western Standard Time", "MS", "America/Montserrat" },
  { "SA Western Standard Time", "PR", "America/Puerto_Rico" },
  { "SA Western Standard Time", "TT", "America/Port_of_Spain" },
  { "SA Western Standard Time", "VC", "America/St_Vincent" },
  { "SA Western Standard Time", "VG", "America/Tortola" },
  { "SA Western Standard Time", "VI", "America/St_Thomas" },
  { "SA Western Standard Time", "ZZ", "Etc/GMT+4" },
  { "SE Asia Standard Time", "", "Asia/Bangkok" },
  { "SE Asia Standard Time", "AQ", "Antarctica/Davis" },
  { "SE Asia Standard Time", "CX", "Indian/Christmas" },
  { "SE Asia Standard Time", "ID", "Asia/Jakarta Asia/Pontianak" },
  { "SE Asia Standard Time", "KH", "Asia/Phnom_Penh" },
  { "SE Asia Standard Time", "LA", "Asia/Vientiane" },
  { "SE Asia Standard Time", "MN", "Asia/Hovd" },
  { "SE Asia Standard Time", "TH", "Asia/Bangkok" },
  { "SE Asia Standard Time", "VN", "Asia/Saigon" },
  { "SE Asia Standard Time", "ZZ", "Etc/GMT-7" },
  { "Samoa Standard Time", "", "Pacific/Apia" },
  { "Samoa Standard Time", "WS", "Pacific/Apia" },
  { "Singapore Standard Time", "", "Asia/Singapore" },
  { "Singapore Standard Time", "BN", "Asia/Brunei" },
  { "Singapore Standard Time", "ID", "Asia/Makassar" },
  { "Singapore Standard Time", "MY", "Asia/Kuala_Lumpur Asia/Kuching" },
  { "Singapore Standard Time", "PH", "Asia/Manila" },
  { "Singapore Standard Time", "SG", "Asia/Singapore" },
  { "Singapore Standard Time", "ZZ", "Etc/GMT-8" },
  { "South Africa Standard Time", "", "Africa/Johannesburg" },
  { "South Africa Standard Time", "BI", "Africa/Bujumbura" },
  { "South Africa Standard Time", "BW", "Africa/Gaborone" },
  { "South Africa Standard Time", "CD", "Africa/Lubumbashi" },
  { "South Africa Standard Time", "LS", "Africa/Maseru" },
  { "South Africa Standard Time", "LY", "Africa/Tripoli" },
  { "South Africa Standard Time", "MW", "Africa/Blantyre" },
  { "South Africa Standard Time", "MZ", "Africa/Maputo" },
  { "South Africa Standard Time", "RW", "Africa/Kigali" },
  { "South Africa Standard Time", "SZ", "Africa/Mbabane" },
  { "South Africa Standard Time", "ZA", "Africa/Johannesburg" },
  { "South Africa Standard Time", "ZM", "Africa/Lusaka" },
  { "South Africa Standard Time", "ZW", "Africa/Harare" },
  { "South Africa Standard Time", "ZZ", "Etc/GMT-2" },
  { "Sri Lanka Standard Time", "", "Asia/Colombo" },
  { "Sri Lanka Standard Time", "LK", "Asia/Colombo" },
  { "Syria Standard Time", "", "Asia/Damascus" },
  { "Syria Standard Time", "SY", "Asia/Damascus" },
  { "Taipei Standard Time", "", "Asia/Taipei" },
  { "Taipei Standard Time", "TW", "Asia/Taipei" },
  { "Tasmania Standard Time", "", "Australia/Hobart" },
  { "Tasmania Standard Time", "AU", "Australia/Hobart Australia/Currie" },
  { "Tokyo Standard Time", "", "Asia/Tokyo" },
  { "Tokyo Standard Time", "ID", "Asia/Jayapura" },
  { "Tokyo Standard Time", "JP", "Asia/Tokyo" },
  { "Tokyo Standard Time", "PW", "Pacific/Palau" },
  { "Tokyo Standard Time", "TL", "Asia/Dili" },
  { "Tokyo Standard Time", "ZZ", "Etc/GMT-9" },
  { "Tonga Standard Time", "", "Pacific/Tongatapu" },
  { "Tonga Standard Time", "KI", "Pacific/Enderbury" },
  { "Tonga Standard Time", "TO", "Pacific/Tongatapu" },
  { "Tonga Standard Time", "ZZ", "Etc/GMT-13" },
  { "Turkey Standard Time", "", "Europe/Istanbul" },
  { "Turkey Standard Time", "TR", "Europe/Istanbul" },
  { "US Eastern Standard Time", "", "America/Indianapolis" },
  { "US Eastern Standard Time", "US", "America/Indianapolis America/Indiana/Marengo America/Indiana/Vevay" },
  { "US Mountain Standard Time", "", "America/Phoenix" },
  { "US Mountain Standard Time", "CA", "America/Dawson_Creek" },
  { "US Mountain Standard Time", "MX", "America/Hermosillo" },
  { "US Mountain Standard Time", "US", "America/Phoenix" },
  { "US Mountain Standard Time", "ZZ", "Etc/GMT+7" },
  { "UTC", "", "Etc/GMT" },
  { "UTC", "GL", "America/Danmarkshavn" },
  { "UTC", "ZZ", "Etc/GMT" },
  { "UTC+12", "", "Etc/GMT-12" },
  { "UTC+12", "KI", "Pacific/Tarawa" },
  { "UTC+12", "MH", "Pacific/Majuro Pacific/Kwajalein" },
  { "UTC+12", "NR", "Pacific/Nauru" },
  { "UTC+12", "TV", "Pacific/Funafuti" },
  { "UTC+12", "UM", "Pacific/Wake" },
  { "UTC+12", "WF", "Pacific/Wallis" },
  { "UTC+12", "ZZ", "Etc/GMT-12" },
  { "UTC-02", "", "Etc/GMT+2" },
  { "UTC-02", "BR", "America/Noronha" },
  { "UTC-02", "GS", "Atlantic/South_Georgia" },
  { "UTC-02", "ZZ", "Etc/GMT+2" },
  { "UTC-11", "", "Etc/GMT+11" },
  { "UTC-11", "AS", "Pacific/Pago_Pago" },
  { "UTC-11", "NU", "Pacific/Niue" },
  { "UTC-11", "UM", "Pacific/Midway" },
  { "UTC-11", "ZZ", "Etc/GMT+11" },
  { "Ulaanbaatar Standard Time", "", "Asia/Ulaanbaatar" },
  { "Ulaanbaatar Standard Time", "MN", "Asia/Ulaanbaatar Asia/Choibalsan" },
  { "Venezuela Standard Time", "", "America/Caracas" },
  { "Venezuela Standard Time", "VE", "America/Caracas" },
  { "Vladivostok Standard Time", "", "Asia/Vladivostok" },
  { "Vladivostok Standard Time", "RU", "Asia/Vladivostok Asia/Sakhalin" },
  { "W. Australia Standard Time", "", "Australia/Perth" },
  { "W. Australia Standard Time", "AQ", "Antarctica/Casey" },
  { "W. Australia Standard Time", "AU", "Australia/Perth" },
  { "W. Central Africa Standard Time", "", "Africa/Lagos" },
  { "W. Central Africa Standard Time", "AO", "Africa/Luanda" },
  { "W. Central Africa Standard Time", "BJ", "Africa/Porto-Novo" },
  { "W. Central Africa Standard Time", "CD", "Africa/Kinshasa" },
  { "W. Central Africa Standard Time", "CF", "Africa/Bangui" },
  { "W. Central Africa Standard Time", "CG", "Africa/Brazzaville" },
  { "W. Central Africa Standard Time", "CM", "Africa/Douala" },
  { "W. Central Africa Standard Time", "DZ", "Africa/Algiers" },
  { "W. Central Africa Standard Time", "GA", "Africa/Libreville" },
  { "W. Central Africa Standard Time", "GQ", "Africa/Malabo" },
  { "W. Central Africa Standard Time", "NE", "Africa/Niamey" },
  { "W. Central Africa Standard Time", "NG", "Africa/Lagos" },
  { "W. Central Africa Standard Time", "TD", "Africa/Ndjamena" },
  { "W. Central Africa Standard Time", "TN", "Africa/Tunis" },
  { "W. Central Africa Standard Time", "ZZ", "Etc/GMT-1" },
  { "W. Europe Standard Time", "", "Europe/Berlin" },
  { "W. Europe Standard Time", "AD", "Europe/Andorra" },
  { "W. Europe Standard Time", "AT", "Europe/Vienna" },
  { "W. Europe Standard Time", "CH", "Europe/Zurich" },
  { "W. Europe Standard Time", "DE", "Europe/Berlin" },
  { "W. Europe Standard Time", "GI", "Europe/Gibraltar" },
  { "W. Europe Standard Time", "IT", "Europe/Rome" },
  { "W. Europe Standard Time", "LI", "Europe/Vaduz" },
  { "W. Europe Standard Time", "LU", "Europe/Luxembourg" },
  { "W. Europe Standard Time", "MC", "Europe/Monaco" },
  { "W. Europe Standard Time", "MT", "Europe/Malta" },
  { "W. Europe Standard Time", "NL", "Europe/Amsterdam" },
  { "W. Europe Standard Time", "NO", "Europe/Oslo" },
  { "W. Europe Standard Time", "SE", "Europe/Stockholm" },
  { "W. Europe Standard Time", "SJ", "Arctic/Longyearbyen" },
  { "W. Europe Standard Time", "SM", "Europe/San_Marino" },
  { "W. Europe Standard Time", "VA", "Europe/Vatican" },
  { "West Asia Standard Time", "", "Asia/Tashkent" },
  { "West Asia Standard Time", "AQ", "Antarctica/Mawson" },
  { "West Asia Standard Time", "KZ", "Asia/Oral Asia/Aqtau Asia/Aqtobe" },
  { "West Asia Standard Time", "MV", "Indian/Maldives" },
  { "West Asia Standard Time", "TF", "Indian/Kerguelen" },
  { "West Asia Standard Time", "TJ", "Asia/Dushanbe" },
  { "West Asia Standard Time", "TM", "Asia/Ashgabat" },
  { "West Asia Standard Time", "UZ", "Asia/Tashkent Asia/Samarkand" },
  { "West Asia Standard Time", "ZZ", "Etc/GMT-5" },
  { "West Pacific Standard Time", "", "Pacific/Port_Moresby" },
  { "West Pacific Standard Time", "AQ", "Antarctica/DumontDUrville" },
  { "West Pacific Standard Time", "FM", "Pacific/Truk" },
  { "West Pacific Standard Time", "GU", "Pacific/Guam" },
  { "West Pacific Standard Time", "MP", "Pacific/Saipan" },
  { "West Pacific Standard Time", "PG", "Pacific/Port_Moresby" },
  { "West Pacific Standard Time", "ZZ", "Etc/GMT-10" },
  { "Yakutsk Standard Time", "", "Asia/Yakutsk" },
  { "Yakutsk Standard Time", "RU", "Asia/Yakutsk" }
};

#define TZMAP_SIZE (sizeof tzmap / sizeof tzmap[0])

static struct option longopts[] =
{
  {"help", no_argument, NULL, 'h' },
  {"version", no_argument, NULL, 'V'},
  {NULL, 0, NULL, 0}
};

static char opts[] = "hV";

#define REG_TZINFO "SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation"
#define REG_TZDB   "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"

/* Not available on pre-XP */
GEOID WINAPI (*getusergeoid)(GEOCLASS);
int WINAPI (*getgeoinfo)(GEOID, GEOTYPE, LPTSTR, int, LANGID);

static inline HKEY
reg_open (HKEY pkey, const char *path, const char *msg)
{
  LONG ret;
  HKEY hkey;

  ret = RegOpenKeyEx (pkey, path, 0, KEY_READ, &hkey);
  if (ret == ERROR_SUCCESS)
    return hkey;
  if (msg)
    fprintf (stderr, "%s: cannot open registry %s, error code %ld\n",
	     program_invocation_short_name, msg, ret);
  return NULL;
}

/* For symmetry */
#define reg_close(hkey)	RegCloseKey(hkey)

static inline BOOL
reg_query (HKEY hkey, const char *value, char *buf, DWORD size, const char *msg)
{
  LONG ret;
  DWORD type;

  ret = RegQueryValueEx (hkey, value, 0, &type, (LPBYTE) buf, &size);
  if (ret == ERROR_SUCCESS)
    return TRUE;
  if (msg)
    fprintf (stderr, "%s: cannot query registry %s, error code %ld\n",
	     program_invocation_short_name, msg, ret);
  return FALSE;
}

static inline BOOL
reg_enum (HKEY hkey, int idx, char *name, DWORD size)
{
  return RegEnumKeyEx (hkey, idx, name, &size, NULL, NULL, NULL, NULL)
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
  char keyname[256], stdname[256], std2name[256], country[10], *spc;
  GEOID geo;
  int opt, idx, gotit = -1;

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
  if (!reg_query (hkey, "TimeZoneKeyName", keyname, sizeof keyname, NULL))
    {
      /* Pre-Vista we have a lot more to do.  First fetch the name of the
	 Standard (non-DST) timezone.  If we can't get that, give up. */
      if (!reg_query (hkey, "StandardName", stdname, sizeof stdname,
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
	      ret = reg_query (skey, stdname[0] == '@' ? "MUI_Std" : "Std",
			       std2name, sizeof std2name, NULL);
	      reg_close (skey);
	      if (ret && !strcmp (stdname, std2name))
		break;
	    }
	}
    }
  reg_close (hkey);

  /* Fetch addresses of Geo functions.  As long as we support Windows 2000
     this is required, unfortunately. */
  getusergeoid = (GEOID (WINAPI *)())
  		 GetProcAddress (GetModuleHandle ("kernel32.dll"),
				 "GetUserGeoID");
  getgeoinfo = (int (WINAPI *)(GEOID, GEOTYPE, LPTSTR, int, LANGID))
	       GetProcAddress (GetModuleHandle ("kernel32.dll"),
			       "GetGeoInfoA");
  *country = '\0';
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
    GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME,
		   country, sizeof country);

  /* Now iterate over the mapping table and find the right entry. */
  for (idx = 0; idx < TZMAP_SIZE; ++idx)
    {
      if (!strcmp (keyname, tzmap[idx].win_tzkey))
	{
	  if (gotit < 0)
	    gotit = idx;
	  if (!strcmp (country, tzmap[idx].country))
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
		   "Windows timezone \"%s\"\n",
		   program_invocation_short_name, keyname);
	  return 1;
	}
      idx = gotit;
    }
  /* Got one.  Print it. */
  spc = strchr (tzmap[idx].posix_tzid, ' ');
  if (spc)
    *spc = '\0';
  printf ("%s\n", tzmap[idx].posix_tzid);
  return 0;
}
