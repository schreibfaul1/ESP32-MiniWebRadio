/*
 * rtime.cpp
 *
 *  Created on: 04.08.2017
 *      Author: Wolle
 *  Updated on: 07.07.2023
 * 
 */

#include "rtime.h"

//    Africa/Addis_Ababa        EAT-3
//    Africa/Algiers            CET-1
//    Africa/Blantyre, Harare   CAT-2
//    Africa/Cairo              EEST
//    Africa/Casablanca         WET0
//    Africa/Freetown           GMT0
//    Africa/Johannesburg       SAST-2
//    Africa/Kinshasa           WAT-1
//    Africa/Lome               GMT0
//    Africa/Maseru             SAST-2
//    Africa/Mbabane            SAST-2
//    Africa/Nairobi            EAT-3
//    Africa/Tripoli            EET-2
//    Africa/Tunis              CET-1CEST,M3.5.0,M10.5.0/3
//    Africa/Windhoek           WAT-1WAST,M9.1.0,M4.1.0
//    America/Adak              HAST10HADT,M3.2.0,M11.1.0
//    America/Alaska            AKST9AKDT,M3.2.0,M11.1.0
//    America/Anguilla,Dominica AST4
//    America/Araguaina         BRT3
//    Argentina/San_Luis        ART3
//    America/Asuncion          PYT4PYST,M10.3.0/0,M3.2.0/0
//    America/Atka              HAST10HADT,M3.2.0,M11.1.0
//    America/Boa_Vista         AMT4
//    America/Bogota            COT5
//    America/Campo_Grande      AMT4AMST,M10.2.0/0,M2.3.0/0
//    America/Caracas           VET4:30
//    America/Catamarca         ART3ARST,M10.1.0/0,M3.3.0/0
//    America/Cayenne           GFT3
//    America/Chicago           CST6CDT,M3.2.0,M11.1.0
//    America/Costa_Rica        CST6
//    America/Los_Angeles       PST8PDT,M3.2.0,M11.1.0
//    America/Dawson_Creek      MST7
//    America/Denver            MST7MDT,M3.2.0,M11.1.0
//    America/Detroit           EST5EDT,M3.2.0,M11.1.0
//    America/Eirunepe          ACT5
//    America/Godthab           WGST
//    America/Guayaquil         ECT5
//    America/Guyana            GYT4
//    America/Havana            CST5CDT,M3.3.0/0,M10.5.0/1
//    America/Hermosillo        MST7
//    America/Jamaica           EST5
//    America/La_Paz            BOT4
//    America/Lima              PET5
//    America/Miquelon          PMST3PMDT,M3.2.0,M11.1.0
//    America/Montevideo        UYT3UYST,M10.1.0,M3.2.0
//    America/Noronha           FNT2
//    America/Paramaribo        SRT3
//    America/Phoenix           MST7
//    America/Santiago          CLST
//    America/Sao_Paulo         BRT3BRST,M10.2.0/0,M2.3.0/0
//    America/Scoresbysund      EGT1EGST,M3.5.0/0,M10.5.0/1
//    America/St_Johns          NST3:30NDT,M3.2.0/0:01,M11.1.0/0:01
//    America/Toronto           EST5EDT,M3.2.0,M11.1.0
//    Antarctica/Casey          WST-8
//    Antarctica/Davis          DAVT-7
//    Antarctica/DumontDUrville DDUT-10
//    Antarctica/Mawson         MAWT-6
//    Antarctica/McMurdo        NZST-12NZDT,M9.5.0,M4.1.0/3
//    Antarctica/Palmer         CLST
//    Antarctica/Rothera        ROTT3
//    Antarctica/South_Pole     NZST-12NZDT,M9.5.0,M4.1.0/3
//    Antarctica/Syowa          SYOT-3
//    Antarctica/Vostok         VOST-6
//    Arctic/Longyearbyen       CET-1CEST,M3.5.0,M10.5.0/3
//    Argentina/Buenos_Aires    ART3ARST,M10.1.0/0,M3.3.0/0
//    Asia/Almaty               ALMT-6
//    Asia/Amman                EET-2EEST,M3.5.4/0,M10.5.5/1
//    Asia/Anadyr               ANAT-12ANAST,M3.5.0,M10.5.0/3
//    Asia/Aqtau, Aqtobe        AQTT-5
//    Asia/Ashgabat             TMT-5
//    Asia/Ashkhabad            TMT-5
//    Asia/Baku                 AZT-4AZST,M3.5.0/4,M10.5.0/5
//    Asia/Bangkok              ICT-7
//    Asia/Bishkek              KGT-6
//    Asia/Brunei               BNT-8
//    Asia/Calcutta             IST-5:30
//    Asia/Choibalsan           CHOT-9
//    Asia/Chongqing            CST-8
//    Asia/Colombo              IST-5:30
//    Asia/Dacca                BDT-6
//    Asia/Damascus             EET-2EEST,M4.1.5/0,J274/0
//    Asia/Dili                 TLT-9
//    Asia/Dubai                GST-4
//    Asia/Dushanbe             TJT-5
//    Asia/Gaza                 EET-2EEST,J91/0,M9.2.4
//    Asia/Ho_Chi_Minh          ICT-7
//    Asia/Hong_Kong            HKT-8
//    Asia/Hovd                 HOVT-7
//    Asia/Irkutsk              IRKT-8IRKST,M3.5.0,M10.5.0/3
//    Asia/Jakarta, Pontianak   WIT-7
//    Asia/Jayapura             EIT-9
//    Asia/Jerusalem            IDDT
//    Asia/Kabul                AFT-4:30
//    Asia/Kamchatka            PETT-12PETST,M3.5.0,M10.5.0/3
//    Asia/Karachi              PKT-5
//    Asia/Katmandu             NPT-5:45
//    Asia/Kolkata              IST-5:30
//    Asia/Krasnoyarsk          KRAT-7KRAST,M3.5.0,M10.5.0/3
//    Asia/Kuala_Lumpur         MYT-8
//    Asia/Kuching              MYT-8
//    Asia/Kuwait, Bahrain      AST-3
//    Asia/Magadan              MAGT-11MAGST,M3.5.0,M10.5.0/3
//    Asia/Makassar             CIT-8
//    Asia/Manila               PHT-8
//    Asia/Mideast/Riyadh87     zzz-3:07:04
//    Asia/Muscat               GST-4
//    Asia/Novosibirsk          NOVT-6NOVST,M3.5.0,M10.5.0/3
//    Asia/Omsk                 OMST-6OMSST,M3.5.0,M10.5.0/3
//    Asia/Oral                 ORAT-5
//    Asia/Phnom_Penh           ICT-7
//    Asia/Pyongyang            KST-9
//    Asia/Qyzylorda            QYZT-6
//    Asia/Rangoon              MMT-6:30
//    Asia/Saigon               ICT-7
//    Asia/Sakhalin             SAKT-10SAKST,M3.5.0,M10.5.0/3
//    Asia/Samarkand            UZT-5
//    Asia/Seoul                KST-9
//    Asia/Singapore            SGT-8
//    Asia/Taipei               CST-8
//    Asia/Tashkent             UZT-5
//    Asia/Tbilisi              GET-4
//    Asia/Tehran               IRDT
//    Asia/Tel_Aviv             IDDT
//    Asia/Thimbu               BTT-6
//    Asia/Thimphu              BTT-6
//    Asia/Tokyo                JST-9
//    Asia/Ujung_Pandang        CIT-8
//    Asia/Ulaanbaatar          ULAT-8
//    Asia/Ulan_Bator           ULAT-8
//    Asia/Urumqi               CST-8
//    Asia/Vientiane            ICT-7
//    Asia/Vladivostok          VLAT-10VLAST,M3.5.0,M10.5.0/3
//    Asia/Yekaterinburg        YAKT-9YAKST,M3.5.0,M10.5.0/3
//    Asia/Yerevan              AMT-4AMST,M3.5.0,M10.5.0/3
//    Atlantic/Azores           AZOT1AZOST,M3.5.0/0,M10.5.0/1
//    Atlantic/Canary           WET0WEST,M3.5.0/1,M10.5.0
//    Atlantic/Cape_Verde       CVT1
//    Atlantic/Jan_Mayen        CET-1CEST,M3.5.0,M10.5.0/3
//    Atlantic/South_Georgia    GST2
//    Atlantic/St_Helena        GMT0
//    Atlantic/Stanley          FKT4FKST,M9.1.0,M4.3.0
//    Australia/Adelaide        CST-9:30CST,M10.1.0,M4.1.0/3
//    Australia/Brisbane        EST-10
//    Australia/Darwin          CST-9:30
//    Australia/Eucla           CWST-8:45
//    Australia/LHI             LHST-10:30LHST-11,M10.1.0,M4.1.0
//    Australia/Lindeman        EST-10
//    Australia/Lord_Howe       LHST-10:30LHST-11,M10.1.0,M4.1.0
//    Australia/Melbourne       EST-10EST,M10.1.0,M4.1.0/3
//    Australia/North           CST-9:30
//    Australia/Perth, West     WST-8
//    Australia/Queensland      EST-10
//    Brazil/Acre               ACT5
//    Brazil/DeNoronha          FNT2
//    Brazil/East               BRT3BRST,M10.2.0/0,M2.3.0/0
//    Brazil/West               AMT4
//    Canada/Central            CST6CDT,M3.2.0,M11.1.0
//    Canada/Eastern            EST5EDT,M3.2.0,M11.1.0
//    Canada/Newfoundland       NST3:30NDT,M3.2.0/0:01,M11.1.0/0:01
//    Canada/Pacific            PST8PDT,M3.2.0,M11.1.0
//    Chile/Continental         CLST
//    Chile/EasterIsland        EASST
//    Europe/Berlin             CET-1CEST,M3.5.0,M10.5.0/3
//    Europe/Athens             EET-2EEST,M3.5.0/3,M10.5.0/4
//    Europe/Belfast            GMT0BST,M3.5.0/1,M10.5.0
//    Europe/Kaliningrad        EET-2EEST,M3.5.0,M10.5.0/3
//    Europe/Lisbon             WET0WEST,M3.5.0/1,M10.5.0
//    Europe/London             GMT0BST,M3.5.0/1,M10.5.0
//    Europe/Minsk              EET-2EEST,M3.5.0,M10.5.0/3
//    Europe/Moscow             MSK-3MSD,M3.5.0,M10.5.0/3
//    Europe/Samara             SAMT-4SAMST,M3.5.0,M10.5.0/3
//    Europe/Volgograd          VOLT-3VOLST,M3.5.0,M10.5.0/3
//    Indian/Chagos             IOT-6
//    Indian/Christmas          CXT-7
//    Indian/Cocos              CCT-6:30
//    Indian/Kerguelen          TFT-5
//    Indian/Mahe               SCT-4
//    Indian/Maldives           MVT-5
//    Indian/Mauritius          MUT-4
//    Indian/Reunion            RET-4
//    Mexico/General            CST6CDT,M4.1.0,M10.5.0
//    Pacific/Apia              WST11
//    Pacific/Auckland          NZST-12NZDT,M9.5.0,M4.1.0/3
//    Pacific/Chatham           CHAST-12:45CHADT,M9.5.0/2:45,M4.1.0/3:45
//    Pacific/Easter            EASST
//    Pacific/Efate             VUT-11
//    Pacific/Enderbury         PHOT-13
//    Pacific/Fakaofo           TKT10
//    Pacific/Fiji              FJT-12
//    Pacific/Funafuti          TVT-12
//    Pacific/Galapagos         GALT6
//    Pacific/Gambier           GAMT9
//    Pacific/Guadalcanal       SBT-11
//    Pacific/Guam              ChST-10
//    Pacific/Honolulu          HST10
//    Pacific/Johnston          HST10
//    Pacific/Kiritimati        LINT-14
//    Pacific/Kosrae            KOST-11
//    Pacific/Kwajalein         MHT-12
//    Pacific/Majuro            MHT-12
//    Pacific/Marquesas         MART9:30
//    Pacific/Midway            SST11
//    Pacific/Nauru             NRT-12
//    Pacific/Niue              NUT11
//    Pacific/Norfolk           NFT-11:30
//    Pacific/Noumea            NCT-11
//    Pacific/Pago_Pago         SST11
//    Pacific/Palau             PWT-9
//    Pacific/Pitcairn          PST8
//    Pacific/Ponape            PONT-11
//    Pacific/Port_Moresby      PGT-10
//    Pacific/Rarotonga         CKT10
//    Pacific/Saipan            ChST-10
//    Pacific/Samoa             SST11
//    Pacific/Tahiti            TAHT10
//    Pacific/Tarawa            GILT-12
//    Pacific/Tongatapu         TOT-13
//    Pacific/Truk              TRUT-10
//    Pacific/Wake              WAKT-12
//    Pacific/Wallis            WFT-12
//    Pacific/Yap               TRUT-10
//    SystemV/HST10             HST10
//    SystemV/MST7              MST7
//    SystemV/PST8              PST8
//    SystemV/YST9              GAMT9
//    US/Aleutian               HAST10HADT,M3.2.0,M11.1.0
//    US/Arizona                MST7
//    US/Eastern                EST5EDT,M3.2.0,M11.1.0
//    US/East-Indiana           EST5EDT,M3.2.0,M11.1.0
//    US/Hawaii                 HST10
//    US/Michigan               EST5EDT,M3.2.0,M11.1.0
//    US/Samoa                  SST11


RTIME::RTIME(){
	timeinfo = { 0,0,0,0,0,0,0,0,0 };
	now=0;
}
RTIME::~RTIME(){
	sntp_stop();
}
boolean RTIME::begin(String TimeZone){
    if(TimeZone.length() == 0) return false;
    RTIME_TZ=TimeZone;
    if (RTIME_info) RTIME_info("Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    char sbuf[20]="pool.ntp.org";
    sntp_setservername(0, sbuf);
    sntp_init();
    return obtain_time();
}

void RTIME::stop(){
    sntp_stop();
}

boolean RTIME::obtain_time(){
    time_t now = 0;
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        sprintf(sbuf, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        if (RTIME_info) RTIME_info(sbuf);
        vTaskDelay(uint16_t(2000 / portTICK_PERIOD_MS));
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    //setenv("TZ","CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1); // automatic daylight saving time
    setenv("TZ", RTIME_TZ.c_str(), 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    if (RTIME_info) RTIME_info(strftime_buf);

    //log_i( "The current date/time in Berlin is: %s", strftime_buf);
    if(retry < retry_count) return true;
    else return false;
}

const char* RTIME::gettime(){
    setenv("TZ", RTIME_TZ.c_str(), 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    //log_i( "The current date/time in Berlin is: %s", strftime_buf);
    return strftime_buf;
}

const char* RTIME::gettime_l(){  // Montag, 04. August 2017 13:12:44
	time(&now);
	localtime_r(&now, &timeinfo);
//    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
//    log_i( "The current date/time in Beriln is: %s", strftime_buf);
	sprintf(strftime_buf,"%s, %02d.%s %d %02d:%02d:%02d",   w_day_l[timeinfo.tm_wday].c_str(), 
                                                            timeinfo.tm_mday, month_l[timeinfo.tm_mon].c_str(),
                                                            timeinfo.tm_year+1900,
                                                            timeinfo.tm_hour,
                                                            timeinfo.tm_min,
                                                            timeinfo.tm_sec);
	return strftime_buf;
}

const char* RTIME::gettime_s(){  // hh:mm:ss
	time(&now);
	localtime_r(&now, &timeinfo);
	sprintf(strftime_buf,"%02d:%02d:%02d",  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	return strftime_buf;
}

const char* RTIME::gettime_xs(){  // hh:mm
    time(&now);
    localtime_r(&now, &timeinfo);
    sprintf(strftime_buf,"%02d:%02d",  timeinfo.tm_hour, timeinfo.tm_min);
    return strftime_buf;
}

const char* RTIME::gettime_xs_12h(){  // hh:mm
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime(strftime_buf, 64, "%I:%M %p",  timeinfo);
    return strftime_buf;
}

uint8_t RTIME::getweekday(){ //So=0, Mo=1 ... Sa=6
    time(&now);
    localtime_r(&now, &timeinfo);
    return timeinfo.tm_wday;
}

uint16_t RTIME::getMinuteOfTheDay(){ // counts at 00:00, from 0...23*60+59
    time(&now);
    localtime_r(&now, &timeinfo);
    return timeinfo.tm_hour * 60 + timeinfo.tm_min;
}
