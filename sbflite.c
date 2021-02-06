<<<<<<< HEAD
#include "sbflite.h"

// FUNCTION PROTOTYPE
void freemem(InverterData *inverters[]);
int getInverterData(InverterData *devList[], enum getInverterDataType type);
void resetInverterData(InverterData *inv);
void CalcMissingSpot(InverterData *invData);
E_SBFSPOT ethGetPacket(void);
E_SBFSPOT ethInitConnectionMulti(InverterData *inverters[], char list[][16]);
E_SBFSPOT logonSMAInverter(InverterData *inverters[], long userGroup, char *password);
E_SBFSPOT logoffSMAInverter(InverterData *inverter);
E_SBFSPOT logoffMultigateDevices(InverterData *inverters[]);
int getInverterIndexBySerial(InverterData *inverters[], unsigned short SUSyID, uint32_t Serial);

int main(int argc, char *argv[])
{
    char msg[80];
    int rc = 0;

    /*************************************************
     * CONFIG
     *************************************************/
    Config cfg;

    cfg.IP_Port = 9522;
    strcpy(cfg.SMA_Password, SMApassword);
    cfg.userGroup = UG_USER;

    if (strlen(cfg.SMA_Password) == 0)
    {
        fprintf(stderr, "Missing USER Password.\n");
        rc = -2;
    }
    if (strlen(cfg.plantname) == 0)
    {
        strncpy(cfg.plantname, SMAplantname, sizeof(cfg.plantname));
    }

    // Allocate memory for InverterData
    InverterData *Inverters[MAX_INVERTERS];
    for (int i = 0; i < MAX_INVERTERS; Inverters[i++] = malloc(sizeof(InverterData)))
        ;

    /*************************************************
     * ETHERNET SETUP
     *************************************************/
    printf("Connecting to Local Network...\n");
    // Ethernet.cpp: Prepares UDP broadcast socket
    rc = ethConnect(cfg.IP_Port);
    if (rc != 0)
    {
        printf("Failed to set up socket connection.\n");
        return rc;
    }

    rc = ethInitConnectionMulti(Inverters, ips);

    if (rc != E_OK)
    {
        printf("Failed to initialize Speedwire connection.\n");
        ethClose();
        return rc;
    }

    printf("\n*************************************************\n* SPEEDWIRE CONNECTION INITIALISED!             *\n*************************************************\n\n");

    /*************************************************
     * SMA LOGIN
     *************************************************/
    if (logonSMAInverter(Inverters, cfg.userGroup, cfg.SMA_Password) != E_OK)
    {
        snprintf(msg, sizeof(msg), "Logon failed. Check '%s' Password\n", cfg.userGroup == UG_USER ? "USER" : "INSTALLER");
        printf("%s\n", msg);
        freemem(Inverters);
        return 1;
    }

    /*************************************************
     * At this point we are logged on to the inverter
     *************************************************/
    puts("Logon OK");

    if ((rc = getInverterData(Inverters, sbftest)) != 0)
        printf("getInverterData(sbftest) returned an error: %d\n", rc);

    if ((rc = getInverterData(Inverters, SoftwareVersion)) != 0)
        printf("getSoftwareVersion returned an error: %d\n", rc);

    if ((rc = getInverterData(Inverters, TypeLabel)) != 0)
        printf("getTypeLabel returned an error: %d\n", rc);
    else
    {
        for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
            printf("Device Name:      %s\n", Inverters[inv]->DeviceName);
            printf("Device Class:     %s%s\n", Inverters[inv]->DeviceClass, (Inverters[inv]->SUSyID == 292) ? " (with battery)" : "");
            printf("Device Type:      %s\n", Inverters[inv]->DeviceType);
            printf("Software Version: %s\n", Inverters[inv]->SWVersion);
            printf("Serial number:    %lu\n", Inverters[inv]->Serial);
        }
    }

    if ((rc = getInverterData(Inverters, DeviceStatus)) != 0)
        printf("getDeviceStatus returned an error: %d\n", rc);
    else
    {
        for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
            //printf("Device Status:      %s\n", getDesc(Inverters[inv]->DeviceStatus, "?"));
        }
    }

    if ((rc = getInverterData(Inverters, InverterTemperature)) != 0)
        printf("getInverterTemperature returned an error: %d\n", rc);
    else
    {
        for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
            printf("Device Temperature: %3.1fC\n", ((float)Inverters[inv]->Temperature / 100)); // degree symbol is different on windows/linux
        }
    }

    if (Inverters[0]->DevClass == SolarInverter)
    {
        if ((rc = getInverterData(Inverters, GridRelayStatus)) != 0)
            printf("getGridRelayStatus returned an error: %d\n", rc);
        else
        {
            for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
            {
                if (Inverters[inv]->DevClass == SolarInverter)
                {
                    printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
                    //printf("GridRelay Status:      %s\n", getDesc(Inverters[inv]->GridRelayStatus, "?"));
                }
            }
        }
    }

    if ((rc = getInverterData(Inverters, MaxACPower)) != 0)
        printf("getMaxACPower returned an error: %d\n", rc);
    else
    {
        //TODO: REVIEW THIS PART (getMaxACPower & getMaxACPower2 should be 1 function)
        if ((Inverters[0]->Pmax1 == 0) && (rc = getInverterData(Inverters, MaxACPower2)) != 0)
            printf("getMaxACPower2 returned an error: %d\n", rc);
        else
        {
            for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
            {
                printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
                printf("Pac max phase 1: %luW\n", Inverters[inv]->Pmax1);
                printf("Pac max phase 2: %luW\n", Inverters[inv]->Pmax2);
                printf("Pac max phase 3: %luW\n", Inverters[inv]->Pmax3);
            }
        }
    }

    if ((rc = getInverterData(Inverters, EnergyProduction)) != 0)
        printf("getEnergyProduction returned an error: %d\n", rc);

    if ((rc = getInverterData(Inverters, OperationTime)) != 0)
        printf("getOperationTime returned an error: %d\n", rc);
    else
    {
        for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
            puts("Energy Production:");
            printf("\tEToday: %.3fkWh\n", tokWh(Inverters[inv]->EToday));
            printf("\tETotal: %.3fkWh\n", tokWh(Inverters[inv]->ETotal));
            printf("\tOperation Time: %.2fh\n", toHour(Inverters[inv]->OperationTime));
            printf("\tFeed-In Time  : %.2fh\n", toHour(Inverters[inv]->FeedInTime));
        }
    }

    if ((rc = getInverterData(Inverters, SpotDCPower)) != 0)
        printf("getSpotDCPower returned an error: %d\n", rc);

    if ((rc = getInverterData(Inverters, SpotDCVoltage)) != 0)
        printf("getSpotDCVoltage returned an error: %d\n", rc);

    //Calculate missing DC Spot Values
    CalcMissingSpot(Inverters[0]);

    for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
    {
        Inverters[inv]->calPdcTot = Inverters[inv]->Pdc1 + Inverters[inv]->Pdc2;
        printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
        puts("DC Spot Data:");
        printf("\tString 1 Pdc: %7.3fkW - Udc: %6.2fV - Idc: %6.3fA\n", tokW(Inverters[inv]->Pdc1), toVolt(Inverters[inv]->Udc1), toAmp(Inverters[inv]->Idc1));
        printf("\tString 2 Pdc: %7.3fkW - Udc: %6.2fV - Idc: %6.3fA\n", tokW(Inverters[inv]->Pdc2), toVolt(Inverters[inv]->Udc2), toAmp(Inverters[inv]->Idc2));
        printf("\tCalculated Total Pdc: %7.3fkW\n", tokW(Inverters[inv]->calPdcTot));
    }

    if ((rc = getInverterData(Inverters, SpotACPower)) != 0)
        printf("getSpotACPower returned an error: %d\n", rc);

    if ((rc = getInverterData(Inverters, SpotACVoltage)) != 0)
        printf("getSpotACVoltage returned an error: %d\n", rc);

    if ((rc = getInverterData(Inverters, SpotACTotalPower)) != 0)
        printf("getSpotACTotalPower returned an error: %d\n", rc);

    //Calculate missing AC Spot Values
    CalcMissingSpot(Inverters[0]);

    for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
    {
        Inverters[inv]->calPacTot = Inverters[inv]->Pac1 + Inverters[inv]->Pac2 + Inverters[inv]->Pac3;
        //Calculated Inverter Efficiency
        Inverters[inv]->calEfficiency = Inverters[inv]->calPdcTot == 0 ? 0.0f : (100.0f * (float)Inverters[inv]->calPacTot / (float)Inverters[inv]->calPdcTot);
        printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
        puts("AC Spot Data:");
        printf("\tPhase 1 Pac : %7.3fkW - Uac: %6.2fV - Iac: %6.3fA\n", tokW(Inverters[inv]->Pac1), toVolt(Inverters[inv]->Uac1), toAmp(Inverters[inv]->Iac1));
        printf("\tPhase 2 Pac : %7.3fkW - Uac: %6.2fV - Iac: %6.3fA\n", tokW(Inverters[inv]->Pac2), toVolt(Inverters[inv]->Uac2), toAmp(Inverters[inv]->Iac2));
        printf("\tPhase 3 Pac : %7.3fkW - Uac: %6.2fV - Iac: %6.3fA\n", tokW(Inverters[inv]->Pac3), toVolt(Inverters[inv]->Uac3), toAmp(Inverters[inv]->Iac3));
        printf("\tTotal Pac   : %7.3fkW - Calculated Pac: %7.3fkW\n", tokW(Inverters[inv]->TotalPac), tokW(Inverters[inv]->calPacTot));
        printf("\tEfficiency  : %7.2f%%\n", Inverters[inv]->calEfficiency);
    }

    if ((rc = getInverterData(Inverters, SpotGridFrequency)) != 0)
        printf("getSpotGridFrequency returned an error: %d\n", rc);
    else
    {
        for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
            printf("Grid Freq. : %.2fHz\n", toHz(Inverters[inv]->GridFreq));
        }
    }

    if (Inverters[0]->DevClass == SolarInverter)
    {
        for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
            if (Inverters[inv]->InverterDatetime > 0)
                printf("Current Inverter Time: %s\n"); //, strftime_t(cfg.DateTimeFormat, Inverters[inv]->InverterDatetime));

            if (Inverters[inv]->WakeupTime > 0)
                printf("Inverter Wake-Up Time: %s\n"); //, strftime_t(cfg.DateTimeFormat, Inverters[inv]->WakeupTime));

            if (Inverters[inv]->SleepTime > 0)
                printf("Inverter Sleep Time  : %s\n"); //, strftime_t(cfg.DateTimeFormat, Inverters[inv]->SleepTime));
        }
    }

    if (Inverters[0]->DevClass == SolarInverter)
    {

        /*
		if ((cfg.CSV_Export == 1) && (cfg.nospot == 0))
			ExportSpotDataToCSV(&cfg, Inverters);

		if (cfg.wsl == 1)
			ExportSpotDataToWSL(&cfg, Inverters);

		if (cfg.s123 == S123_DATA)
			ExportSpotDataTo123s(&cfg, Inverters);
		if (cfg.s123 == S123_INFO)
			ExportInformationDataTo123s(&cfg, Inverters);
		if (cfg.s123 == S123_STATE)
			ExportStateDataTo123s(&cfg, Inverters);
        */
    }

    /*****************
     * Export to SQL *
     *****************/
    // Initialisa C++ object
    db_SQL_Export db = db_SQL_Export();
    // Open DataBase connection
    db.open(sqlHostname, sqlUsername, sqlPassword, sqlDatabase);
    // if dbOpen(sqlHostname, sqlUsername, sqlpassword, sqldatabase??)  Maybe?
    if (db.isopen())
    {
        for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
        {
            /** WARNING: 
              * I was planning on using the time according to the inverter, 
              * however since I also wanted to have a table which counted all wattages 
              * from all inverters. I could not rely on the time according to the Inverter
              **/
            
            // Time according to Inverter[inv]
            //time_t spottime = Inverters[inv]->InverterDatetime;
            // Or time according to server
            time_t spottime = time(NULL);

            char tmpTime[25];
            strcpy(tmpTime, ctime(&spottime));
            // Cut newline char from time string
            tmpTime[strlen(tmpTime) - 1] = '\0';

            db.type_label(Inverters);
            db.device_status(Inverters, spottime);
            db.spot_data(Inverters, spottime);

            time_t datetime = Inverters[inv]->InverterDatetime;
            printf("Current time according to inverter %d: %s\n", inv, ctime(&datetime));

            stringstream sql;
            int rc = SQL_OK;

            for (int i = 0; inv[i] != NULL && i < MAX_INVERTERS; i++)
            {
                sql.str("");
                sql << "INSERT INTO SpotData VALUES(" << spottime << ',' << inv[i]->Serial << ',' << inv[i]->Pdc1 << ',' << inv[i]->Pdc2 << ',' << (float)inv[i]->Idc1 / 1000 << ',' << (float)inv[i]->Idc2 / 1000 << ',' << (float)inv[i]->Udc1 / 100 << ',' << (float)inv[i]->Udc2 / 100 << ',' << inv[i]->Pac1 << ',' << inv[i]->Pac2 << ',' << inv[i]->Pac3 << ',' << (float)inv[i]->Iac1 / 1000 << ',' << (float)inv[i]->Iac2 / 1000 << ',' << (float)inv[i]->Iac3 / 1000 << ',' << (float)inv[i]->Uac1 / 100 << ',' << (float)inv[i]->Uac2 / 100 << ',' << (float)inv[i]->Uac3 / 100 << ',' << inv[i]->EToday << ',' << inv[i]->ETotal << ',' << (float)inv[i]->GridFreq / 100 << ',' << (double)inv[i]->OperationTime / 3600 << ',' << (double)inv[i]->FeedInTime / 3600 << ',' << (float)inv[i]->BT_Signal << ',' << s_quoted(status_text(inv[i]->DeviceStatus)) << ',' << s_quoted(status_text(inv[i]->GridRelayStatus)) << ',' << (float)inv[i]->Temperature / 100 << ")";

                if ((rc = exec_query(sql.str())) != SQL_OK)
                {
                    print_error("[spot_data]exec_query() returned", sql.str());
                    break;
                }
            }
        }
    }
    else
    {
        printf("Could not open connection to database!\n");
    }
    * /

        //SolarInverter -> Continue to get archive data
        unsigned int idx;

    /***************
    * Get Day Data *
    ****************/
    /*
    time_t arch_time = (0 == cfg.startdate) ? time(NULL) : cfg.startdate;

    for (int count=0; count<cfg.archDays; count++)
    {
        if ((rc = ArchiveDayData(Inverters, arch_time)) != E_OK)
        {
            if (rc != E_ARCHNODATA)
		        std::cerr << "ArchiveDayData returned an error: " << rc << std::endl;
        }
        else
        {
            for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
            {
                printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
                for (idx=0; idx<sizeof(Inverters[inv]->dayData)/sizeof(DayData); idx++)
                    if (Inverters[inv]->dayData[idx].datetime > 0)
					{
                        printf("%s : %.3fkWh - %3.3fW\n", strftime_t(cfg.DateTimeFormat, Inverters[inv]->dayData[idx].datetime), (double)Inverters[inv]->dayData[idx].totalWh/1000, (double)Inverters[inv]->dayData[idx].watt);
						fflush(stdout);
					}
                puts("======");
            }

            if (cfg.CSV_Export == 1)
                ExportDayDataToCSV(&cfg, Inverters);

			#if defined(USE_SQLITE) || defined(USE_MYSQL)
			if ((!cfg.nosql) && db.isopen())
				db.day_data(Inverters);
			#endif
        }

        //Goto previous day
        arch_time -= 86400;
    }*/

    /*****************
    * Get Month Data *
    ******************/
    /*
	if (cfg.archMonths > 0)
	{
		getMonthDataOffset(Inverters); //Issues 115/130
		arch_time = (0 == cfg.startdate) ? time(NULL) : cfg.startdate;
		struct tm arch_tm;
		memcpy(&arch_tm, gmtime(&arch_time), sizeof(arch_tm));

		for (int count=0; count<cfg.archMonths; count++)
		{
			ArchiveMonthData(Inverters, &arch_tm);

			if (VERBOSE_HIGH)
			{
				for (int inv = 0; Inverters[inv] != NULL && inv<MAX_INVERTERS; inv++)
				{
					printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
					for (unsigned int ii = 0; ii < sizeof(Inverters[inv]->monthData) / sizeof(MonthData); ii++)
						if (Inverters[inv]->monthData[ii].datetime > 0)
							printf("%s : %.3fkWh - %3.3fkWh\n", strfgmtime_t(cfg.DateFormat, Inverters[inv]->monthData[ii].datetime), (double)Inverters[inv]->monthData[ii].totalWh / 1000, (double)Inverters[inv]->monthData[ii].dayWh / 1000);
					puts("======");
				}
			}

			if (cfg.CSV_Export == 1)
				ExportMonthDataToCSV(&cfg, Inverters);

			#if defined(USE_SQLITE) || defined(USE_MYSQL)
			if ((!cfg.nosql) && db.isopen())
				db.month_data(Inverters);
			#endif

			//Go to previous month
			if (--arch_tm.tm_mon < 0)
			{
				arch_tm.tm_mon = 11;
				arch_tm.tm_year--;
			}
		}
	}*/

    /*****************
    * Get Event Data *
    ******************/
    /*
	posix_time::ptime tm_utc(posix_time::from_time_t((0 == cfg.startdate) ? time(NULL) : cfg.startdate));
	//ptime tm_utc(posix_time::second_clock::universal_time());
	gregorian::date dt_utc(tm_utc.date().year(), tm_utc.date().month(), 1);
	std::string dt_range_csv = str(format("%d%02d") % dt_utc.year() % static_cast<short>(dt_utc.month()));

	for (int m = 0; m < cfg.archEventMonths; m++)
	{
		if (VERBOSE_LOW) cout << "Reading events: " << to_simple_string(dt_utc) << endl;
		//Get user level events
		rc = ArchiveEventData(Inverters, dt_utc, UG_USER);
		if (rc == E_EOF) break; // No more data (first event reached)
		else if (rc != E_OK) std::cerr << "ArchiveEventData(user) returned an error: " << rc << endl;

		//When logged in as installer, get installer level events
		if (cfg.userGroup == UG_INSTALLER)
		{
			rc = ArchiveEventData(Inverters, dt_utc, UG_INSTALLER);
			if (rc == E_EOF) break; // No more data (first event reached)
			else if (rc != E_OK) std::cerr << "ArchiveEventData(installer) returned an error: " << rc << endl;
		}

		//Move to previous month
		if (dt_utc.month() == 1)
			dt_utc = gregorian::date(dt_utc.year() - 1, 12, 1);
		else
			dt_utc = gregorian::date(dt_utc.year(), dt_utc.month() - 1, 1);

	}
		if (rc == E_OK)
	{
		//Adjust start of range with 1 month
		if (dt_utc.month() == 12)
			dt_utc = gregorian::date(dt_utc.year() + 1, 1, 1);
		else
			dt_utc = gregorian::date(dt_utc.year(), dt_utc.month() + 1, 1);
	}

	if ((rc == E_OK) || (rc == E_EOF))
	{
		dt_range_csv = str(format("%d%02d-%s") % dt_utc.year() % static_cast<short>(dt_utc.month()) % dt_range_csv);

		if ((cfg.CSV_Export == 1) && (cfg.archEventMonths > 0))
			ExportEventsToCSV(&cfg, Inverters, dt_range_csv);
	}
    */

    logoffMultigateDevices(Inverters);
    for (int inv = 0; Inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
        logoffSMAInverter(Inverters[inv]);

    freemem(Inverters);

    puts("done.");

    return 0;
}

void HexDump(unsigned char *buf, int count, int radix)
{
    /*
    int i, j;
    printf("--------:");
    for (i=0; i < radix; i++)
    {
        printf(" %02X", i);
    }
    for (i = 0, j = 0; i < count; i++)
    {
        if (j % radix == 0)
        {
            if (radix == 16)
                printf("\n%08X: ", j);
            else
                printf("\n%08d: ", j);
        }
        printf("%02X ", buf[i]);
        j++;
    }
    printf("\n");
	fflush(stdout);
	fflush(stderr);
    */
}

//Free memory allocated by initialiseSMAConnection()
void freemem(InverterData *inverters[])
{
    for (int i = 0; i < MAX_INVERTERS; i++)
        if (inverters[i] != NULL)
        {
            free(inverters[i]);
            inverters[i] = NULL;
        }
}

int getInverterData(InverterData *devList[], enum getInverterDataType type)
{
    printf("getInverterData(%d)\n", type);
    const char *strWatt = "%-12s: %ld (W) %s";
    const char *strVolt = "%-12s: %.2f (V) %s";
    const char *strAmp = "%-12s: %.3f (A) %s";
    const char *strkWh = "%-12s: %.3f (kWh) %s";
    const char *strHour = "%-12s: %.3f (h) %s";

    int rc = E_OK;

    int recordsize = 0;
    int validPcktID = 0;

    unsigned long command;
    unsigned long first;
    unsigned long last;

    switch (type)
    {
    case EnergyProduction:
        // SPOT_ETODAY, SPOT_ETOTAL
        command = 0x54000200;
        first = 0x00260100;
        last = 0x002622FF;
        break;

    case SpotDCPower:
        // SPOT_PDC1, SPOT_PDC2
        command = 0x53800200;
        first = 0x00251E00;
        last = 0x00251EFF;
        break;

    case SpotDCVoltage:
        // SPOT_UDC1, SPOT_UDC2, SPOT_IDC1, SPOT_IDC2
        command = 0x53800200;
        first = 0x00451F00;
        last = 0x004521FF;
        break;

    case SpotACPower:
        // SPOT_PAC1, SPOT_PAC2, SPOT_PAC3
        command = 0x51000200;
        first = 0x00464000;
        last = 0x004642FF;
        break;

    case SpotACVoltage:
        // SPOT_UAC1, SPOT_UAC2, SPOT_UAC3, SPOT_IAC1, SPOT_IAC2, SPOT_IAC3
        command = 0x51000200;
        first = 0x00464800;
        last = 0x004655FF;
        break;

    case SpotGridFrequency:
        // SPOT_FREQ
        command = 0x51000200;
        first = 0x00465700;
        last = 0x004657FF;
        break;

    case MaxACPower:
        // INV_PACMAX1, INV_PACMAX2, INV_PACMAX3
        command = 0x51000200;
        first = 0x00411E00;
        last = 0x004120FF;
        break;

    case MaxACPower2:
        // INV_PACMAX1_2
        command = 0x51000200;
        first = 0x00832A00;
        last = 0x00832AFF;
        break;

    case SpotACTotalPower:
        // SPOT_PACTOT
        command = 0x51000200;
        first = 0x00263F00;
        last = 0x00263FFF;
        break;

    case TypeLabel:
        // INV_NAME, INV_TYPE, INV_CLASS
        command = 0x58000200;
        first = 0x00821E00;
        last = 0x008220FF;
        break;

    case SoftwareVersion:
        // INV_SWVERSION
        command = 0x58000200;
        first = 0x00823400;
        last = 0x008234FF;
        break;

    case DeviceStatus:
        // INV_STATUS
        command = 0x51800200;
        first = 0x00214800;
        last = 0x002148FF;
        break;

    case GridRelayStatus:
        // INV_GRIDRELAY
        command = 0x51800200;
        first = 0x00416400;
        last = 0x004164FF;
        break;

    case OperationTime:
        // SPOT_OPERTM, SPOT_FEEDTM
        command = 0x54000200;
        first = 0x00462E00;
        last = 0x00462FFF;
        break;

    case BatteryChargeStatus:
        command = 0x51000200;
        first = 0x00295A00;
        last = 0x00295AFF;
        break;

    case BatteryInfo:
        command = 0x51000200;
        first = 0x00491E00;
        last = 0x00495DFF;
        break;

    case InverterTemperature:
        command = 0x52000200;
        first = 0x00237700;
        last = 0x002377FF;
        break;

    case sbftest:
        command = 0x64020200;
        first = 0x00618D00;
        last = 0x00618DFF;

    case MeteringGridMsTotW:
        command = 0x51000200;
        first = 0x00463600;
        last = 0x004637FF;
        break;

    default:
        return E_BADARG;
    };

    for (int i = 0; devList[i] != NULL && i < MAX_INVERTERS; i++)
    {
        //do
        //{
        pcktID++;
        writePacketHeader(pcktBuf, 0x01, addr_unknown);
        if (devList[i]->SUSyID == SID_SB240)
            writePacket(pcktBuf, 0x09, 0xE0, 0, devList[i]->SUSyID, devList[i]->Serial);
        else
            writePacket(pcktBuf, 0x09, 0xA0, 0, devList[i]->SUSyID, devList[i]->Serial);
        writeLong(pcktBuf, command);
        writeLong(pcktBuf, first);
        writeLong(pcktBuf, last);
        writePacketTrailer(pcktBuf);
        writePacketLength(pcktBuf);
        //}
        //while (0);

        ethSend(pcktBuf, devList[i]->IPAddress);

        validPcktID = 0;
        do
        {
            rc = ethGetPacket();

            if (rc != E_OK)
                return rc;

            unsigned short rcvpcktID = get_short(pcktBuf + 27) & 0x7FFF;
            if (pcktID == rcvpcktID)
            {
                int inv = getInverterIndexBySerial(devList, get_short(pcktBuf + 15), get_long(pcktBuf + 17));
                if (inv >= 0)
                {
                    validPcktID = 1;
                    int32_t value = 0;
                    int64_t value64 = 0;
                    unsigned char Vtype = 0;
                    unsigned char Vbuild = 0;
                    unsigned char Vminor = 0;
                    unsigned char Vmajor = 0;
                    for (int ii = 41; ii < packetposition - 3; ii += recordsize)
                    {
                        uint32_t code = ((uint32_t)get_long(pcktBuf + ii));
                        LriDef lri = (LriDef)(code & 0x00FFFF00);
                        uint32_t cls = code & 0xFF;
                        unsigned char dataType = code >> 24;
                        time_t datetime = (time_t)get_long(pcktBuf + ii + 4);

                        // fix: We can't rely on dataType because it can be both 0x00 or 0x40 for DWORDs
                        if ((lri == MeteringDyWhOut) || (lri == MeteringTotWhOut) || (lri == MeteringTotFeedTms) || (lri == MeteringTotOpTms)) //QWORD
                        //if ((code == SPOT_ETODAY) || (code == SPOT_ETOTAL) || (code == SPOT_FEEDTM) || (code == SPOT_OPERTM))	//QWORD
                        {
                            value64 = get_longlong(pcktBuf + ii + 8);
                            if ((value64 == (int64_t)NaN_S64) || (value64 == (int64_t)NaN_U64))
                                value64 = 0;
                        }
                        else if ((dataType != 0x10) && (dataType != 0x08)) //Not TEXT or STATUS, so it should be DWORD
                        {
                            value = (int32_t)get_long(pcktBuf + ii + 16);
                            if ((value == (int32_t)NaN_S32) || (value == (int32_t)NaN_U32))
                                value = 0;
                        }

                        switch (lri)
                        {
                        case GridMsTotW: //SPOT_PACTOT
                            if (recordsize == 0)
                                recordsize = 28;
                            //This function gives us the time when the inverter was switched off
                            devList[inv]->SleepTime = datetime;
                            devList[inv]->TotalPac = value;
                            devList[inv]->flags |= type;
                            printf(strWatt, "SPOT_PACTOT", value, ctime(&datetime));
                            break;

                        case OperationHealthSttOk: //INV_PACMAX1
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Pmax1 = value;
                            devList[inv]->flags |= type;
                            printf(strWatt, "INV_PACMAX1", value, ctime(&datetime));
                            break;

                        case OperationHealthSttWrn: //INV_PACMAX2
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Pmax2 = value;
                            devList[inv]->flags |= type;
                            printf(strWatt, "INV_PACMAX2", value, ctime(&datetime));
                            break;

                        case OperationHealthSttAlm: //INV_PACMAX3
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Pmax3 = value;
                            devList[inv]->flags |= type;
                            printf(strWatt, "INV_PACMAX3", value, ctime(&datetime));
                            break;

                        case GridMsWphsA: //SPOT_PAC1
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Pac1 = value;
                            devList[inv]->flags |= type;
                            printf(strWatt, "SPOT_PAC1", value, ctime(&datetime));
                            break;

                        case GridMsWphsB: //SPOT_PAC2
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Pac2 = value;
                            devList[inv]->flags |= type;
                            printf(strWatt, "SPOT_PAC2", value, ctime(&datetime));
                            break;

                        case GridMsWphsC: //SPOT_PAC3
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Pac3 = value;
                            devList[inv]->flags |= type;
                            printf(strWatt, "SPOT_PAC3", value, ctime(&datetime));
                            break;

                        case GridMsPhVphsA: //SPOT_UAC1
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Uac1 = value;
                            devList[inv]->flags |= type;
                            printf(strVolt, "SPOT_UAC1", toVolt(value), ctime(&datetime));
                            break;

                        case GridMsPhVphsB: //SPOT_UAC2
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Uac2 = value;
                            devList[inv]->flags |= type;
                            printf(strVolt, "SPOT_UAC2", toVolt(value), ctime(&datetime));
                            break;

                        case GridMsPhVphsC: //SPOT_UAC3
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Uac3 = value;
                            devList[inv]->flags |= type;
                            printf(strVolt, "SPOT_UAC3", toVolt(value), ctime(&datetime));
                            break;

                        case GridMsAphsA_1: //SPOT_IAC1
                        case GridMsAphsA:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Iac1 = value;
                            devList[inv]->flags |= type;
                            printf(strAmp, "SPOT_IAC1", toAmp(value), ctime(&datetime));
                            break;

                        case GridMsAphsB_1: //SPOT_IAC2
                        case GridMsAphsB:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Iac2 = value;
                            devList[inv]->flags |= type;
                            printf(strAmp, "SPOT_IAC2", toAmp(value), ctime(&datetime));
                            break;

                        case GridMsAphsC_1: //SPOT_IAC3
                        case GridMsAphsC:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Iac3 = value;
                            devList[inv]->flags |= type;
                            printf(strAmp, "SPOT_IAC3", toAmp(value), ctime(&datetime));
                            break;

                        case GridMsHz: //SPOT_FREQ
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->GridFreq = value;
                            devList[inv]->flags |= type;
                            printf("%-12s: %.2f (Hz) %s", "SPOT_FREQ", toHz(value), ctime(&datetime));
                            break;

                        case DcMsWatt: //SPOT_PDC1 / SPOT_PDC2
                            if (recordsize == 0)
                                recordsize = 28;
                            if (cls == 1) // MPP1
                            {
                                devList[inv]->Pdc1 = value;
                                printf(strWatt, "SPOT_PDC1", value, ctime(&datetime));
                            }
                            if (cls == 2) // MPP2
                            {
                                devList[inv]->Pdc2 = value;
                                printf(strWatt, "SPOT_PDC2", value, ctime(&datetime));
                            }
                            devList[inv]->flags |= type;
                            break;

                        case DcMsVol: //SPOT_UDC1 / SPOT_UDC2
                            if (recordsize == 0)
                                recordsize = 28;
                            if (cls == 1)
                            {
                                devList[inv]->Udc1 = value;
                                printf(strVolt, "SPOT_UDC1", toVolt(value), ctime(&datetime));
                            }
                            if (cls == 2)
                            {
                                devList[inv]->Udc2 = value;
                                printf(strVolt, "SPOT_UDC2", toVolt(value), ctime(&datetime));
                            }
                            devList[inv]->flags |= type;
                            break;

                        case DcMsAmp: //SPOT_IDC1 / SPOT_IDC2
                            if (recordsize == 0)
                                recordsize = 28;
                            if (cls == 1)
                            {
                                devList[inv]->Idc1 = value;
                                printf(strAmp, "SPOT_IDC1", toAmp(value), ctime(&datetime));
                            }
                            if (cls == 2)
                            {
                                devList[inv]->Idc2 = value;
                                printf(strAmp, "SPOT_IDC2", toAmp(value), ctime(&datetime));
                            }
                            devList[inv]->flags |= type;
                            break;

                        case MeteringTotWhOut: //SPOT_ETOTAL
                            if (recordsize == 0)
                                recordsize = 16;
                            //In case SPOT_ETODAY missing, this function gives us inverter time (eg: SUNNY TRIPOWER 6.0)
                            devList[inv]->InverterDatetime = datetime;
                            devList[inv]->ETotal = value64;
                            devList[inv]->flags |= type;
                            printf(strkWh, "SPOT_ETOTAL", tokWh(value64), ctime(&datetime));
                            break;

                        case MeteringDyWhOut: //SPOT_ETODAY
                            if (recordsize == 0)
                                recordsize = 16;
                            //This function gives us the current inverter time
                            devList[inv]->InverterDatetime = datetime;
                            devList[inv]->EToday = value64;
                            devList[inv]->flags |= type;
                            printf(strkWh, "SPOT_ETODAY", tokWh(value64), ctime(&datetime));
                            break;

                        case MeteringTotOpTms: //SPOT_OPERTM
                            if (recordsize == 0)
                                recordsize = 16;
                            devList[inv]->OperationTime = value64;
                            devList[inv]->flags |= type;
                            printf(strHour, "SPOT_OPERTM", toHour(value64), ctime(&datetime));
                            break;

                        case MeteringTotFeedTms: //SPOT_FEEDTM
                            if (recordsize == 0)
                                recordsize = 16;
                            devList[inv]->FeedInTime = value64;
                            devList[inv]->flags |= type;
                            printf(strHour, "SPOT_FEEDTM", toHour(value64), ctime(&datetime));
                            break;

                        case NameplateLocation: //INV_NAME
                            if (recordsize == 0)
                                recordsize = 40;
                            //This function gives us the time when the inverter was switched on
                            devList[inv]->WakeupTime = datetime;
                            strncpy(devList[inv]->DeviceName, (char *)pcktBuf + ii + 8, sizeof(devList[inv]->DeviceName) - 1);
                            devList[inv]->flags |= type;
                            printf("%-12s: '%s' %s", "INV_NAME", devList[inv]->DeviceName, ctime(&datetime));
                            break;

                        case NameplatePkgRev: //INV_SWVER
                            if (recordsize == 0)
                                recordsize = 40;
                            Vtype = pcktBuf[ii + 24];
                            char ReleaseType[4];
                            if (Vtype > 5)
                                sprintf(ReleaseType, "%d", Vtype);
                            else
                                sprintf(ReleaseType, "%c", "NEABRS"[Vtype]); //NOREV-EXPERIMENTAL-ALPHA-BETA-RELEASE-SPECIAL
                            Vbuild = pcktBuf[ii + 25];
                            Vminor = pcktBuf[ii + 26];
                            Vmajor = pcktBuf[ii + 27];
                            //Vmajor and Vminor = 0x12 should be printed as '12' and not '18' (BCD)
                            snprintf(devList[inv]->SWVersion, sizeof(devList[inv]->SWVersion), "%c%c.%c%c.%02d.%s", '0' + (Vmajor >> 4), '0' + (Vmajor & 0x0F), '0' + (Vminor >> 4), '0' + (Vminor & 0x0F), Vbuild, ReleaseType);
                            devList[inv]->flags |= type;
                            printf("%-12s: '%s' %s", "INV_SWVER", devList[inv]->SWVersion, ctime(&datetime));
                            break;

                        case NameplateModel: //INV_TYPE
                            if (recordsize == 0)
                                recordsize = 40;
                            for (int idx = 8; idx < recordsize; idx += 4)
                            {
                                unsigned long attribute = ((unsigned long)get_long(pcktBuf + ii + idx)) & 0x00FFFFFF;
                                unsigned char status = pcktBuf[ii + idx + 3];
                                if (attribute == 0xFFFFFE)
                                    break; //End of attributes
                                if (status == 1)
                                {
                                    // TODO: Remove this. I'm too lazy to implement getDesc()
                                    strncpy(devList[inv]->DeviceType, "UNKNOWN TYPE", sizeof(devList[inv]->DeviceType));

                                    /*
										string devtype = getDesc(attribute);
										if (!devtype.empty())
										{
											memset(devList[inv]->DeviceType, 0, sizeof(devList[inv]->DeviceType));
											strncpy(devList[inv]->DeviceType, devtype.c_str(), sizeof(devList[inv]->DeviceType) - 1);
										}
										else
										{
											strncpy(devList[inv]->DeviceType, "UNKNOWN TYPE", sizeof(devList[inv]->DeviceType));
                                            printf("Unknown Inverter Type. Report this issue at https://github.com/SBFspot/SBFspot/issues with following info:\n");
                                            printf("0x%08lX and Inverter Type=<Fill in the exact type> (e.g. SB1300TL-10)\n", attribute);
										}
                                        */
                                }
                            }
                            devList[inv]->flags |= type;
                            printf("%-12s: '%s' %s", "INV_TYPE", devList[inv]->DeviceType, ctime(&datetime));
                            break;

                        case NameplateMainModel: //INV_CLASS
                            if (recordsize == 0)
                                recordsize = 40;
                            for (int idx = 8; idx < recordsize; idx += 4)
                            {
                                unsigned long attribute = ((unsigned long)get_long(pcktBuf + ii + idx)) & 0x00FFFFFF;
                                unsigned char attValue = pcktBuf[ii + idx + 3];
                                if (attribute == 0xFFFFFE)
                                    break; //End of attributes
                                if (attValue == 1)
                                {
                                    devList[inv]->DevClass = (DEVICECLASS)attribute;
                                    // TODO: Remove this. I'm too lazy to implement getDesc()
                                    strncpy(devList[inv]->DeviceClass, "UNKNOWN CLASS", sizeof(devList[inv]->DeviceClass));
                                    /*
										string devclass = getDesc(attribute);
										if (!devclass.empty())
										{
											memset(devList[inv]->DeviceClass, 0, sizeof(devList[inv]->DeviceClass));
											strncpy(devList[inv]->DeviceClass, devclass.c_str(), sizeof(devList[inv]->DeviceClass) - 1);
										}
										else
										{
                                            strncpy(devList[inv]->DeviceClass, "UNKNOWN CLASS", sizeof(devList[inv]->DeviceClass));
                                            printf("Unknown Device Class. Report this issue at https://github.com/SBFspot/SBFspot/issues with following info:\n");
                                            printf("0x%08lX and Device Class=...\n", attribute);
                                        }
                                        */
                                }
                            }
                            devList[inv]->flags |= type;
                            printf("%-12s: '%s' %s", "INV_CLASS", devList[inv]->DeviceClass, ctime(&datetime));
                            break;

                        case OperationHealth: //INV_STATUS:
                            if (recordsize == 0)
                                recordsize = 40;
                            for (int idx = 8; idx < recordsize; idx += 4)
                            {
                                unsigned long attribute = ((unsigned long)get_long(pcktBuf + ii + idx)) & 0x00FFFFFF;
                                unsigned char attValue = pcktBuf[ii + idx + 3];
                                if (attribute == 0xFFFFFE)
                                    break; //End of attributes
                                if (attValue == 1)
                                    devList[inv]->DeviceStatus = attribute;
                            }
                            devList[inv]->flags |= type;
                            //printf("%-12s: '%s' %s", "INV_STATUS", getDesc(devList[inv]->DeviceStatus, "?"), ctime(&datetime));
                            break;

                        case OperationGriSwStt: //INV_GRIDRELAY
                            if (recordsize == 0)
                                recordsize = 40;
                            for (int idx = 8; idx < recordsize; idx += 4)
                            {
                                unsigned long attribute = ((unsigned long)get_long(pcktBuf + ii + idx)) & 0x00FFFFFF;
                                unsigned char attValue = pcktBuf[ii + idx + 3];
                                if (attribute == 0xFFFFFE)
                                    break; //End of attributes
                                if (attValue == 1)
                                    devList[inv]->GridRelayStatus = attribute;
                            }
                            devList[inv]->flags |= type;
                            //printf("%-12s: '%s' %s", "INV_GRIDRELAY", getDesc(devList[inv]->GridRelayStatus, "?"), ctime(&datetime));
                            break;

                        case BatChaStt:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->BatChaStt = value;
                            devList[inv]->flags |= type;
                            break;

                        case BatDiagCapacThrpCnt:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->BatDiagCapacThrpCnt = value;
                            devList[inv]->flags |= type;
                            break;

                        case BatDiagTotAhIn:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->BatDiagTotAhIn = value;
                            devList[inv]->flags |= type;
                            break;

                        case BatDiagTotAhOut:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->BatDiagTotAhOut = value;
                            devList[inv]->flags |= type;
                            break;

                        case BatTmpVal:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->BatTmpVal = value;
                            devList[inv]->flags |= type;
                            break;

                        case BatVol:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->BatVol = value;
                            devList[inv]->flags |= type;
                            break;

                        case BatAmp:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->BatAmp = value;
                            devList[inv]->flags |= type;
                            break;

                        case CoolsysTmpNom:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->Temperature = value;
                            devList[inv]->flags |= type;
                            break;

                        case MeteringGridMsTotWhOut:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->MeteringGridMsTotWOut = value;
                            break;

                        case MeteringGridMsTotWhIn:
                            if (recordsize == 0)
                                recordsize = 28;
                            devList[inv]->MeteringGridMsTotWIn = value;
                            break;

                        default:
                            if (recordsize == 0)
                                recordsize = 12;
                        }
                    }
                }
                else
                {
                    printf("Packet ID mismatch. Expected %d, received %d\n", pcktID, rcvpcktID);
                }
            }
        } while (validPcktID == 0);
    }

    return E_OK;
}

void resetInverterData(InverterData *inv)
{
    inv->BatAmp = 0;
    inv->BatChaStt = 0;
    inv->BatDiagCapacThrpCnt = 0;
    inv->BatDiagTotAhIn = 0;
    inv->BatDiagTotAhOut = 0;
    inv->BatTmpVal = 0;
    inv->BatVol = 0;
    inv->BT_Signal = 0;
    inv->calEfficiency = 0;
    inv->calPacTot = 0;
    inv->calPdcTot = 0;
    inv->DeviceClass[0] = 0;
    inv->DeviceName[0] = 0;
    inv->DeviceStatus = 0;
    inv->DeviceType[0] = 0;
    inv->EToday = 0;
    inv->ETotal = 0;
    inv->FeedInTime = 0;
    inv->flags = 0;
    inv->GridFreq = 0;
    inv->GridRelayStatus = 0;
    inv->Iac1 = 0;
    inv->Iac2 = 0;
    inv->Iac3 = 0;
    inv->Idc1 = 0;
    inv->Idc2 = 0;
    inv->InverterDatetime = 0;
    inv->IPAddress[0] = 0;
    inv->modelID = 0;
    inv->NetID = 0;
    inv->OperationTime = 0;
    inv->Pac1 = 0;
    inv->Pac2 = 0;
    inv->Pac3 = 0;
    inv->Pdc1 = 0;
    inv->Pdc2 = 0;
    inv->Pmax1 = 0;
    inv->Pmax2 = 0;
    inv->Pmax3 = 0;
    inv->Serial = 0;
    inv->SleepTime = 0;
    inv->SUSyID = 0;
    inv->SWVersion[0] = 0;
    inv->Temperature = 0;
    inv->TotalPac = 0;
    inv->Uac1 = 0;
    inv->Uac2 = 0;
    inv->Uac3 = 0;
    inv->Udc1 = 0;
    inv->Udc2 = 0;
    inv->WakeupTime = 0;
    inv->monthDataOffset = 0;
    inv->multigateID = -1;
    inv->MeteringGridMsTotWIn = 0;
    inv->MeteringGridMsTotWOut = 0;
}

//Power Values are missing on some inverters
void CalcMissingSpot(InverterData *invData)
{
    if (invData->Pdc1 == 0)
        invData->Pdc1 = (invData->Idc1 * invData->Udc1) / 100000;
    if (invData->Pdc2 == 0)
        invData->Pdc2 = (invData->Idc2 * invData->Udc2) / 100000;

    if (invData->Pac1 == 0)
        invData->Pac1 = (invData->Iac1 * invData->Uac1) / 100000;
    if (invData->Pac2 == 0)
        invData->Pac2 = (invData->Iac2 * invData->Uac2) / 100000;
    if (invData->Pac3 == 0)
        invData->Pac3 = (invData->Iac3 * invData->Uac3) / 100000;

    if (invData->TotalPac == 0)
        invData->TotalPac = invData->Pac1 + invData->Pac2 + invData->Pac3;
}

E_SBFSPOT ethGetPacket(void)
{
    printf("ethGetPacket()\n");
    E_SBFSPOT rc = E_OK;

    ethPacketHeaderL1L2 *pkHdr = (ethPacketHeaderL1L2 *)CommBuf;

    do
    {
        int bib = ethRead(CommBuf, sizeof(CommBuf));

        if (bib <= 0)
        {
            printf("No data!\n");
            rc = E_NODATA;
        }
        else
        {
            unsigned short pkLen = (pkHdr->pcktHdrL1.hiPacketLen << 8) + pkHdr->pcktHdrL1.loPacketLen;

            //More data after header?
            if (pkLen > 0)
            {
                HexDump(CommBuf, bib, 10);
                if (btohl(pkHdr->pcktHdrL2.MagicNumber) == ETH_L2SIGNATURE)
                {
                    // Copy CommBuf to packetbuffer
                    // Dummy byte to align with BTH (7E)
                    pcktBuf[0] = 0;
                    // We need last 6 bytes of ethPacketHeader too
                    memcpy(pcktBuf + 1, CommBuf + sizeof(ethPacketHeaderL1), bib - sizeof(ethPacketHeaderL1));
                    // Point packetposition at last byte in our buffer
                    // This is different from BTH
                    packetposition = bib - sizeof(ethPacketHeaderL1);

                    printf("<<<====== Content of pcktBuf =======>>>\n");
                    HexDump(pcktBuf, packetposition, 10);
                    printf("<<<=================================>>>\n");

                    rc = E_OK;
                }
                else
                {
                    printf("L2 header not found.\n");
                    rc = E_RETRY;
                }
            }
            else
                rc = E_NODATA;
        }
    } while (rc == E_RETRY);

    return rc;
}

// Initialise multiple ethernet connected inverters
E_SBFSPOT ethInitConnectionMulti(InverterData *inverters[], char list[][16])
{
    puts("Initializing...");

    //Generate a Serial Number for application
    AppSUSyID = 125;
    srand(time(NULL));
    AppSerial = 900000000 + ((rand() << 16) + rand()) % 100000000;

    printf("SUSyID: %d - SessionID: %lu\n", AppSUSyID, AppSerial);

    E_SBFSPOT rc = E_OK;

    for (unsigned int devcount = 0; devcount < num_inverters; devcount++)
    {
        inverters[devcount] = malloc(sizeof(InverterData));
        resetInverterData(inverters[devcount]);
        strcpy(inverters[devcount]->IPAddress, ips[devcount]);
        printf("Inverter IP address: %s\n", inverters[devcount]->IPAddress);

        writePacketHeader(pcktBuf, 0, NULL);
        writePacket(pcktBuf, 0x09, 0xA0, 0, anySUSyID, anySerial);
        writeLong(pcktBuf, 0x00000200);
        writeLong(pcktBuf, 0);
        writeLong(pcktBuf, 0);
        writeLong(pcktBuf, 0);
        writePacketLength(pcktBuf);

        ethSend(pcktBuf, inverters[devcount]->IPAddress);

        if ((rc = ethGetPacket()) == E_OK)
        {
            ethPacket *pckt = (ethPacket *)pcktBuf;
            inverters[devcount]->SUSyID = btohs(pckt->Source.SUSyID);
            inverters[devcount]->Serial = btohl(pckt->Source.Serial);
            printf("Inverter replied: %s SUSyID: %d - Serial: %lu\n", inverters[devcount]->IPAddress, inverters[devcount]->SUSyID, inverters[devcount]->Serial);
        }
        else
        {
            printf("ERROR: Connection to inverter failed!\n");
            printf("Is %s the correct IP?\n", inverters[devcount]->IPAddress);
            return E_INIT;
        }

        logoffSMAInverter(inverters[devcount]);
    }

    return rc;
}

E_SBFSPOT logonSMAInverter(InverterData *inverters[], long userGroup, char *password)
{
#define MAX_PWLENGTH 12
    unsigned char pw[MAX_PWLENGTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    puts("logonSMAInverter()");

    char encChar = (userGroup == UG_USER) ? 0x88 : 0xBB;
    //Encode password
    unsigned int idx;
    for (idx = 0; (password[idx] != 0) && (idx < sizeof(pw)); idx++)
        pw[idx] = password[idx] + encChar;
    for (; idx < MAX_PWLENGTH; idx++)
        pw[idx] = encChar;

    E_SBFSPOT rc = E_OK;
    int validPcktID = 0;

    time_t now;

    for (int inv = 0; inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
    {
        printf("Logining into %s\n", inverters[inv]->IPAddress);
        do
        {
            pcktID++;
            now = time(NULL);
            writePacketHeader(pcktBuf, 0x01, addr_unknown);
            if (inverters[inv]->SUSyID != SID_SB240)
                writePacket(pcktBuf, 0x0E, 0xA0, 0x0100, inverters[inv]->SUSyID, inverters[inv]->Serial);
            else
                writePacket(pcktBuf, 0x0E, 0xE0, 0x0100, inverters[inv]->SUSyID, inverters[inv]->Serial);
            writeLong(pcktBuf, 0xFFFD040C);
            writeLong(pcktBuf, userGroup);  // User / Installer
            writeLong(pcktBuf, 0x00000384); // Timeout = 900sec ?
            writeLong(pcktBuf, now);
            writeLong(pcktBuf, 0);
            writeArray(pcktBuf, pw, sizeof(pw));
            writePacketTrailer(pcktBuf);
            writePacketLength(pcktBuf);
        } while (!1);

        ethSend(pcktBuf, inverters[inv]->IPAddress);
        validPcktID = 0;

        do
        {
            if ((rc = ethGetPacket()) == E_OK)
            {
                ethPacket *pckt = (ethPacket *)pcktBuf;
                if (pcktID == (btohs(pckt->PacketID) & 0x7FFF)) // Valid Packet ID
                {
                    validPcktID = 1;
                    unsigned short retcode = btohs(pckt->ErrorCode);
                    switch (retcode)
                    {
                    case 0:
                        rc = E_OK;
                        break;
                    case 0x0100:
                        rc = E_INVPASSW;
                        break;
                    default:
                        rc = E_LOGONFAILED;
                        break;
                    }
                }
                else
                    printf("Packet ID mismatch. Expected %d, received %d\n", pcktID, (btohs(pckt->PacketID) & 0x7FFF));
            }
        } while ((validPcktID == 0) && (rc == E_OK)); // Fix Issue 167
    }
    return rc;
}

E_SBFSPOT logoffSMAInverter(InverterData *inverter)
{
    puts("logoffSMAInverter()");
    do
    {
        pcktID++;
        writePacketHeader(pcktBuf, 0x01, addr_unknown);
        writePacket(pcktBuf, 0x08, 0xA0, 0x0300, anySUSyID, anySerial);
        writeLong(pcktBuf, 0xFFFD010E);
        writeLong(pcktBuf, 0xFFFFFFFF);
        writePacketTrailer(pcktBuf);
        writePacketLength(pcktBuf);
    } while (!1);

    ethSend(pcktBuf, inverter->IPAddress);

    return E_OK;
}

E_SBFSPOT logoffMultigateDevices(InverterData *inverters[])
{
    puts("logoffMultigateDevices()");
    for (int mg = 0; inverters[mg] != NULL && mg < MAX_INVERTERS; mg++)
    {
        InverterData *pmg = inverters[mg];
        if (pmg->SUSyID == SID_MULTIGATE)
        {
            pmg->hasDayData = 1; //true;
            for (int sb240 = 0; inverters[sb240] != NULL && sb240 < MAX_INVERTERS; sb240++)
            {
                InverterData *psb = inverters[sb240];
                if ((psb->SUSyID == SID_SB240) && (psb->multigateID == mg))
                {
                    //do
                    //{
                    pcktID++;
                    writePacketHeader(pcktBuf, 0, NULL);
                    writePacket(pcktBuf, 0x08, 0xE0, 0x0300, psb->SUSyID, psb->Serial);
                    writeLong(pcktBuf, 0xFFFD010E);
                    writeLong(pcktBuf, 0xFFFFFFFF);
                    writePacketTrailer(pcktBuf);
                    writePacketLength(pcktBuf);
                    //}
                    //while (!isCrcValid(pcktBuf[packetposition-3], pcktBuf[packetposition-2]));

                    ethSend(pcktBuf, psb->IPAddress);

                    psb->logonStatus = 0; // logged of

                    printf("Logoff %d:%d\n", psb->SUSyID, psb->Serial);
                }
            }
        }
    }

    return E_OK;
}

// Ethernet.c //
const char *IP_Broadcast = "239.12.255.254";

int ethConnect(short port)
{
    int ret = 0;
    // create socket for UDP
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("Socket error : %i\n", sock);
        return -1;
    }

    // set up parameters for UDP listen socket
    memset((char *)&addr_out, 0, sizeof(addr_out));
    addr_out.sin_family = AF_INET;
    addr_out.sin_port = htons(port);
    addr_out.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind this port (on any interface) to our UDP listener
    ret = bind(sock, (struct sockaddr *)&addr_out, sizeof(addr_out));
    // here is the destination IP
    addr_out.sin_addr.s_addr = inet_addr(IP_Broadcast);

    // Set options to receive broadcast packets
    struct ip_mreq mreq;

    mreq.imr_multiaddr.s_addr = inet_addr(IP_Broadcast);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq));
    unsigned char loop = 0;
    ret = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (const char *)&loop, sizeof(loop));

    if (ret < 0)
    {
        printf("setsockopt IP_ADD_MEMBERSHIP failed\n");
        return -1;
    }
    // end of setting broadcast options

    return 0; //OK
}

int ethRead(unsigned char *buf, unsigned int bufsize)
{
    int bytes_read;
    short timeout = 5;
    socklen_t addr_in_len = sizeof(addr_in);

    fd_set readfds;

    do
    {
        struct timeval tv;
        tv.tv_sec = timeout; //set timeout of reading
        tv.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        int rc = select(sock + 1, &readfds, NULL, NULL, &tv);
        printf("select() returned %d\n", rc);
        if (rc == -1)
        {
            printf("errno = %d\n", errno);
        }

        if (FD_ISSET(sock, &readfds))
            bytes_read = recvfrom(sock, (char *)buf, bufsize, 0, (struct sockaddr *)&addr_in, &addr_in_len);
        else
        {
            puts("Timeout reading socket");
            return -1;
        }

        if (bytes_read > 0)
        {
            if (bytes_read > MAX_CommBuf)
            {
                MAX_CommBuf = bytes_read;
                printf("MAX_CommBuf is now %d bytes\n", MAX_CommBuf);
            }
            printf("Received %d bytes from IP [%s]\n", bytes_read, inet_ntoa(addr_in.sin_addr));
            if (bytes_read == 600 || bytes_read == 608 || bytes_read == 0)
                printf(" ==> packet ignored\n");
        }
        else
            printf("recvfrom() returned an error: %d\n", bytes_read);

    } while (bytes_read == 600 || bytes_read == 608); // keep on reading if data received from Energy Meter (600 bytes) or Sunny Home Manager (608 bytes)

    return bytes_read;
}

int ethSend(unsigned char *buffer, const char *toIP)
{
    HexDump(buffer, packetposition, 10);

    addr_out.sin_addr.s_addr = inet_addr(toIP);
    size_t bytes_sent = sendto(sock, (const char *)buffer, packetposition, 0, (struct sockaddr *)&addr_out, sizeof(addr_out));

    printf(" %d Bytes sent to IP [%s]\n", bytes_sent, inet_ntoa(addr_out.sin_addr));
    return bytes_sent;
}

int ethClose()
{
    if (sock != 0)
    {
        close(sock);
        sock = 0;
    }
    return 0;
}

int getLocalIP(unsigned char IPaddress[4])
{
    int rc = 0;

    struct ifaddrs *myaddrs;
    struct in_addr *inaddr;

    if (getifaddrs(&myaddrs) == 0)
    {
        for (struct ifaddrs *ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr != NULL)
            {
                // Find the active network adapter
                if ((ifa->ifa_addr->sa_family == AF_INET) && (ifa->ifa_flags & IFF_UP) && (strcmp(ifa->ifa_name, "lo") != 0))
                {
                    struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                    inaddr = &s4->sin_addr;

                    unsigned long ipaddr = inaddr->s_addr;
                    IPaddress[3] = ipaddr & 0xFF;
                    IPaddress[2] = (ipaddr >> 8) & 0xFF;
                    IPaddress[1] = (ipaddr >> 16) & 0xFF;
                    IPaddress[0] = (ipaddr >> 24) & 0xFF;

                    break;
                }
            }
        }

        freeifaddrs(myaddrs);
    }
    else
        rc = -1;

    return rc;
}

void writeLong(BYTE *btbuffer, unsigned long v)
{
    writeByte(btbuffer, (unsigned char)((v >> 0) & 0xFF));
    writeByte(btbuffer, (unsigned char)((v >> 8) & 0xFF));
    writeByte(btbuffer, (unsigned char)((v >> 16) & 0xFF));
    writeByte(btbuffer, (unsigned char)((v >> 24) & 0xFF));
}

void writeShort(BYTE *btbuffer, unsigned short v)
{
    writeByte(btbuffer, (unsigned char)((v >> 0) & 0xFF));
    writeByte(btbuffer, (unsigned char)((v >> 8) & 0xFF));
}

void writeByte(unsigned char *btbuffer, unsigned char v)
{
    btbuffer[packetposition++] = v;
}

void writeArray(unsigned char *btbuffer, const unsigned char bytes[], int loopcount)
{
    for (int i = 0; i < loopcount; i++)
    {
        writeByte(btbuffer, bytes[i]);
    }
}

void writePacket(unsigned char *buf, unsigned char longwords, unsigned char ctrl, unsigned short ctrl2, unsigned short dstSUSyID, unsigned long dstSerial)
{
    writeLong(buf, ETH_L2SIGNATURE);

    writeByte(buf, longwords);
    writeByte(buf, ctrl);
    writeShort(buf, dstSUSyID);
    writeLong(buf, dstSerial);
    writeShort(buf, ctrl2);
    writeShort(buf, AppSUSyID);
    writeLong(buf, AppSerial);
    writeShort(buf, ctrl2);
    writeShort(buf, 0);
    writeShort(buf, 0);
    writeShort(buf, pcktID | 0x8000);
}

void writePacketTrailer(unsigned char *btbuffer)
{
    writeLong(btbuffer, 0);
}

void writePacketHeader(unsigned char *buf, const unsigned int control, const unsigned char *destaddress)
{
    packetposition = 0;

    //Ignore control and destaddress
    writeLong(buf, 0x00414D53); // SMA\0
    writeLong(buf, 0xA0020400);
    writeLong(buf, 0x01000000);
    writeByte(buf, 0);
    writeByte(buf, 0); // Placeholder for packet length
}

void writePacketLength(unsigned char *buf)
{
    short dataLength = (short)(packetposition - sizeof(ethPacketHeaderL1L2));
    ethPacketHeaderL1L2 *hdr = (ethPacketHeaderL1L2 *)buf;
    hdr->pcktHdrL1.hiPacketLen = (dataLength >> 8) & 0xFF;
    hdr->pcktHdrL1.loPacketLen = dataLength & 0xFF;
}

int64_t get_longlong(BYTE *buf)
{
    register int64_t lnglng = 0;

    lnglng += *(buf + 7);
    lnglng <<= 8;
    lnglng += *(buf + 6);
    lnglng <<= 8;
    lnglng += *(buf + 5);
    lnglng <<= 8;
    lnglng += *(buf + 4);
    lnglng <<= 8;
    lnglng += *(buf + 3);
    lnglng <<= 8;
    lnglng += *(buf + 2);
    lnglng <<= 8;
    lnglng += *(buf + 1);
    lnglng <<= 8;
    lnglng += *(buf);

    return lnglng;
}

int32_t get_long(BYTE *buf)
{
    register int32_t lng = 0;

    lng += *(buf + 3);
    lng <<= 8;
    lng += *(buf + 2);
    lng <<= 8;
    lng += *(buf + 1);
    lng <<= 8;
    lng += *(buf);

    return lng;
}

short get_short(BYTE *buf)
{
    register short shrt = 0;

    shrt += *(buf + 1);
    shrt <<= 8;
    shrt += *(buf);

    return shrt;
}

int getInverterIndexBySerial(InverterData *inverters[], unsigned short SUSyID, uint32_t Serial)
{
    printf("getInverterIndexBySerial()\n");
    printf("Looking up %d:%lu\n", SUSyID, (unsigned long)Serial);

    for (int inv = 0; inverters[inv] != NULL && inv < MAX_INVERTERS; inv++)
    {
        printf("Inverter[%d] %d:%lu\n", inv, inverters[inv]->SUSyID, inverters[inv]->Serial);

        if ((inverters[inv]->SUSyID == SUSyID) && inverters[inv]->Serial == Serial)
            return inv;
    }

    printf("Serial Not Found!\n");

    return -1;
}
=======
#include "sbflite.h"

// FUNCTION PROTOTYPE
void freemem(InverterData *inverters[]);
int getInverterData(InverterData *devList[], enum getInverterDataType type);
void resetInverterData(InverterData *inv);
void CalcMissingSpot(InverterData *invData);
E_SBFSPOT ethGetPacket(void);
E_SBFSPOT ethInitConnectionMulti(InverterData *inverters[], char list[][16]);
E_SBFSPOT logonSMAInverter(InverterData *inverters[], long userGroup, char *password);
E_SBFSPOT logoffSMAInverter(InverterData *inverter);
E_SBFSPOT logoffMultigateDevices(InverterData *inverters[]);
int getInverterIndexBySerial(InverterData *inverters[], unsigned short SUSyID, uint32_t Serial);

#define num_inverters 2
char ips[num_inverters][16]= {
    "192.168.1.51",
    "192.168.1.52",
};

int main(int argc, char *argv[])
{
    char msg[80];
    int rc = 0;

    /*************************************************
     * CONFIG
     *************************************************/
    Config cfg;

    cfg.IP_Port = 9522;
    strcpy(cfg.SMA_Password, "PASSWORD");
	cfg.userGroup = UG_USER;

    if (strlen(cfg.SMA_Password) == 0)
    {
        fprintf(stderr, "Missing USER Password.\n");
        rc = -2;
    }
    if (strlen(cfg.plantname) == 0)
    {
        strncpy(cfg.plantname, "MyPlant", sizeof(cfg.plantname));
    }


    // Allocate memory for InverterData
    InverterData *Inverters[MAX_INVERTERS];
    for (int i=0; i<MAX_INVERTERS; Inverters[i++] = malloc(sizeof(InverterData)));

    /*************************************************
     * ETHERNET SETUP
     *************************************************/
    printf("Connecting to Local Network...\n");
	// Ethernet.cpp: Prepares UDP broadcast socket
	rc = ethConnect(cfg.IP_Port);
	if (rc != 0)
	{
		printf("Failed to set up socket connection.\n");
		return rc;
	}
    
	rc = ethInitConnectionMulti(Inverters, ips);

	if (rc != E_OK)
	{
		printf("Failed to initialize Speedwire connection.\n");
		ethClose();
		return rc;
	}

	printf(
"\n*************************************************\n* SPEEDWIRE CONNECTION INITIALISED!             *\n*************************************************\n\n");

    /*************************************************
     * SMA LOGIN
     *************************************************/
    if (logonSMAInverter(Inverters, cfg.userGroup, cfg.SMA_Password) != E_OK)
    {
        snprintf(msg, sizeof(msg), "Logon failed. Check '%s' Password\n", cfg.userGroup == UG_USER? "USER":"INSTALLER");
        printf("%s\n", msg);
        freemem(Inverters);
        return 1;
    }

    /*************************************************
     * At this point we are logged on to the inverter
     *************************************************/
	puts("Logon OK");

	if ((rc = getInverterData(Inverters, sbftest)) != 0)
        printf("getInverterData(sbftest) returned an error: %d\n",rc);

	if ((rc = getInverterData(Inverters, SoftwareVersion)) != 0)
        printf("getSoftwareVersion returned an error: %d\n",rc);

    if ((rc = getInverterData(Inverters, TypeLabel)) != 0)
        printf("getTypeLabel returned an error: %d\n",rc);
    else
    {
        for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
            printf("Device Name:      %s\n", Inverters[inv]->DeviceName);
            printf("Device Class:     %s%s\n", Inverters[inv]->DeviceClass, (Inverters[inv]->SUSyID == 292) ? " (with battery)":"");
            printf("Device Type:      %s\n", Inverters[inv]->DeviceType);
            printf("Software Version: %s\n", Inverters[inv]->SWVersion);
            printf("Serial number:    %lu\n", Inverters[inv]->Serial);
        }
    }

    if ((rc = getInverterData(Inverters, DeviceStatus)) != 0)
        printf("getDeviceStatus returned an error: %d\n",rc);
    else
    {
        for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
			//printf("Device Status:      %s\n", getDesc(Inverters[inv]->DeviceStatus, "?"));
        }
    }

	if ((rc = getInverterData(Inverters, InverterTemperature)) != 0)
        printf("getInverterTemperature returned an error: %d\n",rc);
    else
    {
        for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
		    printf("Device Temperature: %3.1fC\n", ((float)Inverters[inv]->Temperature / 100)); // degree symbol is different on windows/linux
        }
    }

	if (Inverters[0]->DevClass == SolarInverter)
    {
        if ((rc = getInverterData(Inverters, GridRelayStatus)) != 0)
            printf("getGridRelayStatus returned an error: %d\n",rc);
        else
        {
            for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
            {
                if (Inverters[inv]->DevClass == SolarInverter)
                {
                    printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
				    //printf("GridRelay Status:      %s\n", getDesc(Inverters[inv]->GridRelayStatus, "?"));
                }
            }
        }
    }

    if ((rc = getInverterData(Inverters, MaxACPower)) != 0)
        printf("getMaxACPower returned an error: %d\n",rc);
    else
    {
        //TODO: REVIEW THIS PART (getMaxACPower & getMaxACPower2 should be 1 function)
        if ((Inverters[0]->Pmax1 == 0) && (rc = getInverterData(Inverters, MaxACPower2)) != 0)
	        printf("getMaxACPower2 returned an error: %d\n",rc);
        else
        {
            for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
            {
                printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
                printf("Pac max phase 1: %luW\n", Inverters[inv]->Pmax1);
                printf("Pac max phase 2: %luW\n", Inverters[inv]->Pmax2);
                printf("Pac max phase 3: %luW\n", Inverters[inv]->Pmax3);
            }
        }
    }

    if ((rc = getInverterData(Inverters, EnergyProduction)) != 0)
        printf("getEnergyProduction returned an error: %d\n",rc);

    if ((rc = getInverterData(Inverters, OperationTime)) != 0)
        printf("getOperationTime returned an error: %d\n",rc);
    else
    {
        for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
            puts("Energy Production:");
            printf("\tEToday: %.3fkWh\n", tokWh(Inverters[inv]->EToday));
            printf("\tETotal: %.3fkWh\n", tokWh(Inverters[inv]->ETotal));
            printf("\tOperation Time: %.2fh\n", toHour(Inverters[inv]->OperationTime));
            printf("\tFeed-In Time  : %.2fh\n", toHour(Inverters[inv]->FeedInTime));
        }
    }

    if ((rc = getInverterData(Inverters, SpotDCPower)) != 0)
        printf("getSpotDCPower returned an error: %d\n",rc);

    if ((rc = getInverterData(Inverters, SpotDCVoltage)) != 0)
        printf("getSpotDCVoltage returned an error: %d\n",rc);

    //Calculate missing DC Spot Values
    CalcMissingSpot(Inverters[0]);

    for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
    {
		Inverters[inv]->calPdcTot = Inverters[inv]->Pdc1 + Inverters[inv]->Pdc2;
        printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
        puts("DC Spot Data:");
        printf("\tString 1 Pdc: %7.3fkW - Udc: %6.2fV - Idc: %6.3fA\n", tokW(Inverters[inv]->Pdc1), toVolt(Inverters[inv]->Udc1), toAmp(Inverters[inv]->Idc1));
        printf("\tString 2 Pdc: %7.3fkW - Udc: %6.2fV - Idc: %6.3fA\n", tokW(Inverters[inv]->Pdc2), toVolt(Inverters[inv]->Udc2), toAmp(Inverters[inv]->Idc2));
        printf("\tCalculated Total Pdc: %7.3fkW\n", tokW(Inverters[inv]->calPdcTot));
    }

    if ((rc = getInverterData(Inverters, SpotACPower)) != 0)
        printf("getSpotACPower returned an error: %d\n",rc);

    if ((rc = getInverterData(Inverters, SpotACVoltage)) != 0)
        printf("getSpotACVoltage returned an error: %d\n",rc);

    if ((rc = getInverterData(Inverters, SpotACTotalPower)) != 0)
       printf("getSpotACTotalPower returned an error: %d\n",rc);

    //Calculate missing AC Spot Values
    CalcMissingSpot(Inverters[0]);

    for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
    {
        Inverters[inv]->calPacTot = Inverters[inv]->Pac1 + Inverters[inv]->Pac2 + Inverters[inv]->Pac3;
        //Calculated Inverter Efficiency
        Inverters[inv]->calEfficiency = Inverters[inv]->calPdcTot == 0 ? 0.0f : (100.0f * (float)Inverters[inv]->calPacTot / (float)Inverters[inv]->calPdcTot);
        printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
        puts("AC Spot Data:");
        printf("\tPhase 1 Pac : %7.3fkW - Uac: %6.2fV - Iac: %6.3fA\n", tokW(Inverters[inv]->Pac1), toVolt(Inverters[inv]->Uac1), toAmp(Inverters[inv]->Iac1));
        printf("\tPhase 2 Pac : %7.3fkW - Uac: %6.2fV - Iac: %6.3fA\n", tokW(Inverters[inv]->Pac2), toVolt(Inverters[inv]->Uac2), toAmp(Inverters[inv]->Iac2));
        printf("\tPhase 3 Pac : %7.3fkW - Uac: %6.2fV - Iac: %6.3fA\n", tokW(Inverters[inv]->Pac3), toVolt(Inverters[inv]->Uac3), toAmp(Inverters[inv]->Iac3));
        printf("\tTotal Pac   : %7.3fkW - Calculated Pac: %7.3fkW\n", tokW(Inverters[inv]->TotalPac), tokW(Inverters[inv]->calPacTot));
        printf("\tEfficiency  : %7.2f%%\n", Inverters[inv]->calEfficiency);
    }

    if ((rc = getInverterData(Inverters, SpotGridFrequency)) != 0)
        printf("getSpotGridFrequency returned an error: %d\n", rc);
    else
    {
        for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
        {
            printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
            printf("Grid Freq. : %.2fHz\n", toHz(Inverters[inv]->GridFreq));
        }
    }

    if (Inverters[0]->DevClass == SolarInverter)
	{
		for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
		{
			printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
			if (Inverters[inv]->InverterDatetime > 0)
				printf("Current Inverter Time: %s\n");//, strftime_t(cfg.DateTimeFormat, Inverters[inv]->InverterDatetime));

			if (Inverters[inv]->WakeupTime > 0)
				printf("Inverter Wake-Up Time: %s\n");//, strftime_t(cfg.DateTimeFormat, Inverters[inv]->WakeupTime));

			if (Inverters[inv]->SleepTime > 0)
				printf("Inverter Sleep Time  : %s\n");//, strftime_t(cfg.DateTimeFormat, Inverters[inv]->SleepTime));
		}
	}

	if (Inverters[0]->DevClass == SolarInverter)
	{

        /*
		if ((cfg.CSV_Export == 1) && (cfg.nospot == 0))
			ExportSpotDataToCSV(&cfg, Inverters);

		if (cfg.wsl == 1)
			ExportSpotDataToWSL(&cfg, Inverters);

		if (cfg.s123 == S123_DATA)
			ExportSpotDataTo123s(&cfg, Inverters);
		if (cfg.s123 == S123_INFO)
			ExportInformationDataTo123s(&cfg, Inverters);
		if (cfg.s123 == S123_STATE)
			ExportStateDataTo123s(&cfg, Inverters);
        */
	}

	//SolarInverter -> Continue to get archive data
	unsigned int idx;

    /***************
    * Get Day Data *
    ****************/
   /*
    time_t arch_time = (0 == cfg.startdate) ? time(NULL) : cfg.startdate;

    for (int count=0; count<cfg.archDays; count++)
    {
        if ((rc = ArchiveDayData(Inverters, arch_time)) != E_OK)
        {
            if (rc != E_ARCHNODATA)
		        std::cerr << "ArchiveDayData returned an error: " << rc << std::endl;
        }
        else
        {
            for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
            {
                printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
                for (idx=0; idx<sizeof(Inverters[inv]->dayData)/sizeof(DayData); idx++)
                    if (Inverters[inv]->dayData[idx].datetime > 0)
					{
                        printf("%s : %.3fkWh - %3.3fW\n", strftime_t(cfg.DateTimeFormat, Inverters[inv]->dayData[idx].datetime), (double)Inverters[inv]->dayData[idx].totalWh/1000, (double)Inverters[inv]->dayData[idx].watt);
						fflush(stdout);
					}
                puts("======");
            }

            if (cfg.CSV_Export == 1)
                ExportDayDataToCSV(&cfg, Inverters);

			#if defined(USE_SQLITE) || defined(USE_MYSQL)
			if ((!cfg.nosql) && db.isopen())
				db.day_data(Inverters);
			#endif
        }

        //Goto previous day
        arch_time -= 86400;
    }*/


    /*****************
    * Get Month Data *
    ******************/
   /*
	if (cfg.archMonths > 0)
	{
		getMonthDataOffset(Inverters); //Issues 115/130
		arch_time = (0 == cfg.startdate) ? time(NULL) : cfg.startdate;
		struct tm arch_tm;
		memcpy(&arch_tm, gmtime(&arch_time), sizeof(arch_tm));

		for (int count=0; count<cfg.archMonths; count++)
		{
			ArchiveMonthData(Inverters, &arch_tm);

			if (VERBOSE_HIGH)
			{
				for (int inv = 0; Inverters[inv] != NULL && inv<MAX_INVERTERS; inv++)
				{
					printf("SUSyID: %d - SN: %lu\n", Inverters[inv]->SUSyID, Inverters[inv]->Serial);
					for (unsigned int ii = 0; ii < sizeof(Inverters[inv]->monthData) / sizeof(MonthData); ii++)
						if (Inverters[inv]->monthData[ii].datetime > 0)
							printf("%s : %.3fkWh - %3.3fkWh\n", strfgmtime_t(cfg.DateFormat, Inverters[inv]->monthData[ii].datetime), (double)Inverters[inv]->monthData[ii].totalWh / 1000, (double)Inverters[inv]->monthData[ii].dayWh / 1000);
					puts("======");
				}
			}

			if (cfg.CSV_Export == 1)
				ExportMonthDataToCSV(&cfg, Inverters);

			#if defined(USE_SQLITE) || defined(USE_MYSQL)
			if ((!cfg.nosql) && db.isopen())
				db.month_data(Inverters);
			#endif

			//Go to previous month
			if (--arch_tm.tm_mon < 0)
			{
				arch_tm.tm_mon = 11;
				arch_tm.tm_year--;
			}
		}
	}*/

    /*****************
    * Get Event Data *
    ******************/
    /*
	posix_time::ptime tm_utc(posix_time::from_time_t((0 == cfg.startdate) ? time(NULL) : cfg.startdate));
	//ptime tm_utc(posix_time::second_clock::universal_time());
	gregorian::date dt_utc(tm_utc.date().year(), tm_utc.date().month(), 1);
	std::string dt_range_csv = str(format("%d%02d") % dt_utc.year() % static_cast<short>(dt_utc.month()));

	for (int m = 0; m < cfg.archEventMonths; m++)
	{
		if (VERBOSE_LOW) cout << "Reading events: " << to_simple_string(dt_utc) << endl;
		//Get user level events
		rc = ArchiveEventData(Inverters, dt_utc, UG_USER);
		if (rc == E_EOF) break; // No more data (first event reached)
		else if (rc != E_OK) std::cerr << "ArchiveEventData(user) returned an error: " << rc << endl;

		//When logged in as installer, get installer level events
		if (cfg.userGroup == UG_INSTALLER)
		{
			rc = ArchiveEventData(Inverters, dt_utc, UG_INSTALLER);
			if (rc == E_EOF) break; // No more data (first event reached)
			else if (rc != E_OK) std::cerr << "ArchiveEventData(installer) returned an error: " << rc << endl;
		}

		//Move to previous month
		if (dt_utc.month() == 1)
			dt_utc = gregorian::date(dt_utc.year() - 1, 12, 1);
		else
			dt_utc = gregorian::date(dt_utc.year(), dt_utc.month() - 1, 1);

	}
		if (rc == E_OK)
	{
		//Adjust start of range with 1 month
		if (dt_utc.month() == 12)
			dt_utc = gregorian::date(dt_utc.year() + 1, 1, 1);
		else
			dt_utc = gregorian::date(dt_utc.year(), dt_utc.month() + 1, 1);
	}

	if ((rc == E_OK) || (rc == E_EOF))
	{
		dt_range_csv = str(format("%d%02d-%s") % dt_utc.year() % static_cast<short>(dt_utc.month()) % dt_range_csv);

		if ((cfg.CSV_Export == 1) && (cfg.archEventMonths > 0))
			ExportEventsToCSV(&cfg, Inverters, dt_range_csv);
	}
    */


	logoffMultigateDevices(Inverters);
	for (int inv=0; Inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
		logoffSMAInverter(Inverters[inv]);

    freemem(Inverters);

    puts("done.");

    return 0;
}

void HexDump(unsigned char *buf, int count, int radix)
{
    /*
    int i, j;
    printf("--------:");
    for (i=0; i < radix; i++)
    {
        printf(" %02X", i);
    }
    for (i = 0, j = 0; i < count; i++)
    {
        if (j % radix == 0)
        {
            if (radix == 16)
                printf("\n%08X: ", j);
            else
                printf("\n%08d: ", j);
        }
        printf("%02X ", buf[i]);
        j++;
    }
    printf("\n");
	fflush(stdout);
	fflush(stderr);
    */
}

//Free memory allocated by initialiseSMAConnection()
void freemem(InverterData *inverters[])
{
    for (int i=0; i<MAX_INVERTERS; i++)
        if (inverters[i] != NULL)
        {
            free(inverters[i]);
            inverters[i] = NULL;
        }
}

int getInverterData(InverterData *devList[], enum getInverterDataType type)
{
    printf("getInverterData(%d)\n", type);
    const char *strWatt = "%-12s: %ld (W) %s";
    const char *strVolt = "%-12s: %.2f (V) %s";
    const char *strAmp = "%-12s: %.3f (A) %s";
    const char *strkWh = "%-12s: %.3f (kWh) %s";
    const char *strHour = "%-12s: %.3f (h) %s";

    int rc = E_OK;

    int recordsize = 0;
    int validPcktID = 0;

    unsigned long command;
    unsigned long first;
    unsigned long last;

    switch(type)
    {
    case EnergyProduction:
        // SPOT_ETODAY, SPOT_ETOTAL
        command = 0x54000200;
        first = 0x00260100;
        last = 0x002622FF;
        break;

    case SpotDCPower:
        // SPOT_PDC1, SPOT_PDC2
        command = 0x53800200;
        first = 0x00251E00;
        last = 0x00251EFF;
        break;

    case SpotDCVoltage:
        // SPOT_UDC1, SPOT_UDC2, SPOT_IDC1, SPOT_IDC2
        command = 0x53800200;
        first = 0x00451F00;
        last = 0x004521FF;
        break;

    case SpotACPower:
        // SPOT_PAC1, SPOT_PAC2, SPOT_PAC3
        command = 0x51000200;
        first = 0x00464000;
        last = 0x004642FF;
        break;

    case SpotACVoltage:
        // SPOT_UAC1, SPOT_UAC2, SPOT_UAC3, SPOT_IAC1, SPOT_IAC2, SPOT_IAC3
        command = 0x51000200;
        first = 0x00464800;
        last = 0x004655FF;
        break;

    case SpotGridFrequency:
        // SPOT_FREQ
        command = 0x51000200;
        first = 0x00465700;
        last = 0x004657FF;
        break;

    case MaxACPower:
        // INV_PACMAX1, INV_PACMAX2, INV_PACMAX3
        command = 0x51000200;
        first = 0x00411E00;
        last = 0x004120FF;
        break;

    case MaxACPower2:
        // INV_PACMAX1_2
        command = 0x51000200;
        first = 0x00832A00;
        last = 0x00832AFF;
        break;

    case SpotACTotalPower:
        // SPOT_PACTOT
        command = 0x51000200;
        first = 0x00263F00;
        last = 0x00263FFF;
        break;

    case TypeLabel:
        // INV_NAME, INV_TYPE, INV_CLASS
        command = 0x58000200;
        first = 0x00821E00;
        last = 0x008220FF;
        break;

    case SoftwareVersion:
        // INV_SWVERSION
        command = 0x58000200;
        first = 0x00823400;
        last = 0x008234FF;
        break;

    case DeviceStatus:
        // INV_STATUS
        command = 0x51800200;
        first = 0x00214800;
        last = 0x002148FF;
        break;

    case GridRelayStatus:
        // INV_GRIDRELAY
        command = 0x51800200;
        first = 0x00416400;
        last = 0x004164FF;
        break;

    case OperationTime:
        // SPOT_OPERTM, SPOT_FEEDTM
        command = 0x54000200;
        first = 0x00462E00;
        last = 0x00462FFF;
        break;

    case BatteryChargeStatus:
        command = 0x51000200;
        first = 0x00295A00;
        last = 0x00295AFF;
        break;

    case BatteryInfo:
        command = 0x51000200;
        first = 0x00491E00;
        last = 0x00495DFF;
        break;

	case InverterTemperature:
		command = 0x52000200;
		first = 0x00237700;
		last = 0x002377FF;
		break;

	case sbftest:
		command = 0x64020200;
		first = 0x00618D00;
		last = 0x00618DFF;

	case MeteringGridMsTotW:
		command = 0x51000200;
		first = 0x00463600;
		last = 0x004637FF;
		break;

    default:
        return E_BADARG;
    };

    for (int i=0; devList[i]!=NULL && i<MAX_INVERTERS; i++)
    {
		//do
		//{
			pcktID++;
			writePacketHeader(pcktBuf, 0x01, addr_unknown);
			if (devList[i]->SUSyID == SID_SB240)
				writePacket(pcktBuf, 0x09, 0xE0, 0, devList[i]->SUSyID, devList[i]->Serial);
			else
				writePacket(pcktBuf, 0x09, 0xA0, 0, devList[i]->SUSyID, devList[i]->Serial);
			writeLong(pcktBuf, command);
			writeLong(pcktBuf, first);
			writeLong(pcktBuf, last);
			writePacketTrailer(pcktBuf);
			writePacketLength(pcktBuf);
		//}
		//while (0);

		ethSend(pcktBuf, devList[i]->IPAddress);

		validPcktID = 0;
        do
        {
            rc = ethGetPacket();

            if (rc != E_OK) return rc;

            unsigned short rcvpcktID = get_short(pcktBuf+27) & 0x7FFF;
            if (pcktID == rcvpcktID)
            {
                    int inv = getInverterIndexBySerial(devList, get_short(pcktBuf + 15), get_long(pcktBuf + 17));
                    if (inv >= 0)
                    {
                        validPcktID = 1;
                        int32_t value = 0;
                        int64_t value64 = 0;
                        unsigned char Vtype = 0;
                        unsigned char Vbuild = 0;
                        unsigned char Vminor = 0;
                        unsigned char Vmajor = 0;
                        for (int ii = 41; ii < packetposition - 3; ii += recordsize)
                        {
                            uint32_t code = ((uint32_t)get_long(pcktBuf + ii));
                            LriDef lri = (LriDef)(code & 0x00FFFF00);
                            uint32_t cls = code & 0xFF;
                            unsigned char dataType = code >> 24;
                            time_t datetime = (time_t)get_long(pcktBuf + ii + 4);

                            // fix: We can't rely on dataType because it can be both 0x00 or 0x40 for DWORDs
                            if ((lri == MeteringDyWhOut) || (lri == MeteringTotWhOut) || (lri == MeteringTotFeedTms) || (lri == MeteringTotOpTms))	//QWORD
                            //if ((code == SPOT_ETODAY) || (code == SPOT_ETOTAL) || (code == SPOT_FEEDTM) || (code == SPOT_OPERTM))	//QWORD
                            {
                                value64 = get_longlong(pcktBuf + ii + 8);
                                if ((value64 == (int64_t)NaN_S64) || (value64 == (int64_t)NaN_U64)) value64 = 0;
                            }
                            else if ((dataType != 0x10) && (dataType != 0x08))	//Not TEXT or STATUS, so it should be DWORD
                            {
                                value = (int32_t)get_long(pcktBuf + ii + 16);
                                if ((value == (int32_t)NaN_S32) || (value == (int32_t)NaN_U32)) value = 0;
                            }

                            switch (lri)
                            {
                            case GridMsTotW: //SPOT_PACTOT
                                if (recordsize == 0) recordsize = 28;
                                //This function gives us the time when the inverter was switched off
                                devList[inv]->SleepTime = datetime;
                                devList[inv]->TotalPac = value;
                                devList[inv]->flags |= type;
                                printf(strWatt, "SPOT_PACTOT", value, ctime(&datetime));
                                break;

                            case OperationHealthSttOk: //INV_PACMAX1
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Pmax1 = value;
                                devList[inv]->flags |= type;
                                printf(strWatt, "INV_PACMAX1", value, ctime(&datetime));
                                break;

                            case OperationHealthSttWrn: //INV_PACMAX2
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Pmax2 = value;
                                devList[inv]->flags |= type;
                                printf(strWatt, "INV_PACMAX2", value, ctime(&datetime));
                                break;

                            case OperationHealthSttAlm: //INV_PACMAX3
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Pmax3 = value;
                                devList[inv]->flags |= type;
                                printf(strWatt, "INV_PACMAX3", value, ctime(&datetime));
                                break;

                            case GridMsWphsA: //SPOT_PAC1
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Pac1 = value;
                                devList[inv]->flags |= type;
                                printf(strWatt, "SPOT_PAC1", value, ctime(&datetime));
                                break;

                            case GridMsWphsB: //SPOT_PAC2
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Pac2 = value;
                                devList[inv]->flags |= type;
                                printf(strWatt, "SPOT_PAC2", value, ctime(&datetime));
                                break;

                            case GridMsWphsC: //SPOT_PAC3
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Pac3 = value;
                                devList[inv]->flags |= type;
                                printf(strWatt, "SPOT_PAC3", value, ctime(&datetime));
                                break;

                            case GridMsPhVphsA: //SPOT_UAC1
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Uac1 = value;
                                devList[inv]->flags |= type;
                                printf(strVolt, "SPOT_UAC1", toVolt(value), ctime(&datetime));
                                break;

                            case GridMsPhVphsB: //SPOT_UAC2
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Uac2 = value;
                                devList[inv]->flags |= type;
                                printf(strVolt, "SPOT_UAC2", toVolt(value), ctime(&datetime));
                                break;

                            case GridMsPhVphsC: //SPOT_UAC3
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Uac3 = value;
                                devList[inv]->flags |= type;
                                printf(strVolt, "SPOT_UAC3", toVolt(value), ctime(&datetime));
                                break;

                            case GridMsAphsA_1: //SPOT_IAC1
							case GridMsAphsA:
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Iac1 = value;
                                devList[inv]->flags |= type;
                                printf(strAmp, "SPOT_IAC1", toAmp(value), ctime(&datetime));
                                break;

                            case GridMsAphsB_1: //SPOT_IAC2
							case GridMsAphsB:
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Iac2 = value;
                                devList[inv]->flags |= type;
                                printf(strAmp, "SPOT_IAC2", toAmp(value), ctime(&datetime));
                                break;

                            case GridMsAphsC_1: //SPOT_IAC3
							case GridMsAphsC:
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->Iac3 = value;
                                devList[inv]->flags |= type;
                                printf(strAmp, "SPOT_IAC3", toAmp(value), ctime(&datetime));
                                break;

                            case GridMsHz: //SPOT_FREQ
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->GridFreq = value;
                                devList[inv]->flags |= type;
                                printf("%-12s: %.2f (Hz) %s", "SPOT_FREQ", toHz(value), ctime(&datetime));
                                break;

                            case DcMsWatt: //SPOT_PDC1 / SPOT_PDC2
                                if (recordsize == 0) recordsize = 28;
                                if (cls == 1)   // MPP1
                                {
                                    devList[inv]->Pdc1 = value;
                                    printf(strWatt, "SPOT_PDC1", value, ctime(&datetime));
                                }
                                if (cls == 2)   // MPP2
                                {
                                    devList[inv]->Pdc2 = value;
                                    printf(strWatt, "SPOT_PDC2", value, ctime(&datetime));
                                }
                                devList[inv]->flags |= type;
                                break;

                            case DcMsVol: //SPOT_UDC1 / SPOT_UDC2
                                if (recordsize == 0) recordsize = 28;
                                if (cls == 1)
                                {
                                    devList[inv]->Udc1 = value;
                                    printf(strVolt, "SPOT_UDC1", toVolt(value), ctime(&datetime));
                                }
                                if (cls == 2)
                                {
                                    devList[inv]->Udc2 = value;
                                    printf(strVolt, "SPOT_UDC2", toVolt(value), ctime(&datetime));
                                }
                                devList[inv]->flags |= type;
                                break;

                            case DcMsAmp: //SPOT_IDC1 / SPOT_IDC2
                                if (recordsize == 0) recordsize = 28;
                                if (cls == 1)
                                {
                                    devList[inv]->Idc1 = value;
                                    printf(strAmp, "SPOT_IDC1", toAmp(value), ctime(&datetime));
                                }
                                if (cls == 2)
                                {
                                    devList[inv]->Idc2 = value;
                                    printf(strAmp, "SPOT_IDC2", toAmp(value), ctime(&datetime));
                                }
                                devList[inv]->flags |= type;
                                break;

                            case MeteringTotWhOut: //SPOT_ETOTAL
                                if (recordsize == 0) recordsize = 16;
								//In case SPOT_ETODAY missing, this function gives us inverter time (eg: SUNNY TRIPOWER 6.0)
								devList[inv]->InverterDatetime = datetime;
								devList[inv]->ETotal = value64;
                                devList[inv]->flags |= type;
                                printf(strkWh, "SPOT_ETOTAL", tokWh(value64), ctime(&datetime));
                                break;

                            case MeteringDyWhOut: //SPOT_ETODAY
                                if (recordsize == 0) recordsize = 16;
                                //This function gives us the current inverter time
                                devList[inv]->InverterDatetime = datetime;
                                devList[inv]->EToday = value64;
                                devList[inv]->flags |= type;
                                printf(strkWh, "SPOT_ETODAY", tokWh(value64), ctime(&datetime));
                                break;

                            case MeteringTotOpTms: //SPOT_OPERTM
                                if (recordsize == 0) recordsize = 16;
                                devList[inv]->OperationTime = value64;
                                devList[inv]->flags |= type;
                                printf(strHour, "SPOT_OPERTM", toHour(value64), ctime(&datetime));
                                break;

                            case MeteringTotFeedTms: //SPOT_FEEDTM
                                if (recordsize == 0) recordsize = 16;
                                devList[inv]->FeedInTime = value64;
                                devList[inv]->flags |= type;
                                printf(strHour, "SPOT_FEEDTM", toHour(value64), ctime(&datetime));
                                break;

                            case NameplateLocation: //INV_NAME
                                if (recordsize == 0) recordsize = 40;
                                //This function gives us the time when the inverter was switched on
                                devList[inv]->WakeupTime = datetime;
                                strncpy(devList[inv]->DeviceName, (char *)pcktBuf + ii + 8, sizeof(devList[inv]->DeviceName)-1);
                                devList[inv]->flags |= type;
                                printf("%-12s: '%s' %s", "INV_NAME", devList[inv]->DeviceName, ctime(&datetime));
                                break;

                            case NameplatePkgRev: //INV_SWVER
                                if (recordsize == 0) recordsize = 40;
                                Vtype = pcktBuf[ii + 24];
                                char ReleaseType[4];
                                if (Vtype > 5)
                                    sprintf(ReleaseType, "%d", Vtype);
                                else
                                    sprintf(ReleaseType, "%c", "NEABRS"[Vtype]); //NOREV-EXPERIMENTAL-ALPHA-BETA-RELEASE-SPECIAL
                                Vbuild = pcktBuf[ii + 25];
                                Vminor = pcktBuf[ii + 26];
                                Vmajor = pcktBuf[ii + 27];
                                //Vmajor and Vminor = 0x12 should be printed as '12' and not '18' (BCD)
                                snprintf(devList[inv]->SWVersion, sizeof(devList[inv]->SWVersion), "%c%c.%c%c.%02d.%s", '0'+(Vmajor >> 4), '0'+(Vmajor & 0x0F), '0'+(Vminor >> 4), '0'+(Vminor & 0x0F), Vbuild, ReleaseType);
                                devList[inv]->flags |= type;
                                printf("%-12s: '%s' %s", "INV_SWVER", devList[inv]->SWVersion, ctime(&datetime));
                                break;

                            case NameplateModel: //INV_TYPE
                                if (recordsize == 0) recordsize = 40;
                                for (int idx = 8; idx < recordsize; idx += 4)
                                {
                                    unsigned long attribute = ((unsigned long)get_long(pcktBuf + ii + idx)) & 0x00FFFFFF;
                                    unsigned char status = pcktBuf[ii + idx + 3];
                                    if (attribute == 0xFFFFFE) break;	//End of attributes
                                    if (status == 1)
                                    {
                                        // TODO: Remove this. I'm too lazy to implement getDesc()
                                        strncpy(devList[inv]->DeviceType, "UNKNOWN TYPE", sizeof(devList[inv]->DeviceType));

                                        /*
										string devtype = getDesc(attribute);
										if (!devtype.empty())
										{
											memset(devList[inv]->DeviceType, 0, sizeof(devList[inv]->DeviceType));
											strncpy(devList[inv]->DeviceType, devtype.c_str(), sizeof(devList[inv]->DeviceType) - 1);
										}
										else
										{
											strncpy(devList[inv]->DeviceType, "UNKNOWN TYPE", sizeof(devList[inv]->DeviceType));
                                            printf("Unknown Inverter Type. Report this issue at https://github.com/SBFspot/SBFspot/issues with following info:\n");
                                            printf("0x%08lX and Inverter Type=<Fill in the exact type> (e.g. SB1300TL-10)\n", attribute);
										}
                                        */
                                    }
                                }
                                devList[inv]->flags |= type;
                                printf("%-12s: '%s' %s", "INV_TYPE", devList[inv]->DeviceType, ctime(&datetime));
                                break;

                            case NameplateMainModel: //INV_CLASS
                                if (recordsize == 0) recordsize = 40;
                                for (int idx = 8; idx < recordsize; idx += 4)
                                {
                                    unsigned long attribute = ((unsigned long)get_long(pcktBuf + ii + idx)) & 0x00FFFFFF;
                                    unsigned char attValue = pcktBuf[ii + idx + 3];
                                    if (attribute == 0xFFFFFE) break;	//End of attributes
                                    if (attValue == 1)
                                    {
                                        devList[inv]->DevClass = (DEVICECLASS)attribute;
                                        // TODO: Remove this. I'm too lazy to implement getDesc()
                                        strncpy(devList[inv]->DeviceClass, "UNKNOWN CLASS", sizeof(devList[inv]->DeviceClass));
                                        /*
										string devclass = getDesc(attribute);
										if (!devclass.empty())
										{
											memset(devList[inv]->DeviceClass, 0, sizeof(devList[inv]->DeviceClass));
											strncpy(devList[inv]->DeviceClass, devclass.c_str(), sizeof(devList[inv]->DeviceClass) - 1);
										}
										else
										{
                                            strncpy(devList[inv]->DeviceClass, "UNKNOWN CLASS", sizeof(devList[inv]->DeviceClass));
                                            printf("Unknown Device Class. Report this issue at https://github.com/SBFspot/SBFspot/issues with following info:\n");
                                            printf("0x%08lX and Device Class=...\n", attribute);
                                        }
                                        */
                                    }
                                }
                                devList[inv]->flags |= type;
                                printf("%-12s: '%s' %s", "INV_CLASS", devList[inv]->DeviceClass, ctime(&datetime));
                                break;

                            case OperationHealth: //INV_STATUS:
                                if (recordsize == 0) recordsize = 40;
                                for (int idx = 8; idx < recordsize; idx += 4)
                                {
                                    unsigned long attribute = ((unsigned long)get_long(pcktBuf + ii + idx)) & 0x00FFFFFF;
                                    unsigned char attValue = pcktBuf[ii + idx + 3];
                                    if (attribute == 0xFFFFFE) break;	//End of attributes
                                    if (attValue == 1)
                                        devList[inv]->DeviceStatus = attribute;
                                }
                                devList[inv]->flags |= type;
								//printf("%-12s: '%s' %s", "INV_STATUS", getDesc(devList[inv]->DeviceStatus, "?"), ctime(&datetime));
                                break;

                            case OperationGriSwStt: //INV_GRIDRELAY
                                if (recordsize == 0) recordsize = 40;
                                for (int idx = 8; idx < recordsize; idx += 4)
                                {
                                    unsigned long attribute = ((unsigned long)get_long(pcktBuf + ii + idx)) & 0x00FFFFFF;
                                    unsigned char attValue = pcktBuf[ii + idx + 3];
                                    if (attribute == 0xFFFFFE) break;	//End of attributes
                                    if (attValue == 1)
                                        devList[inv]->GridRelayStatus = attribute;
                                }
                                devList[inv]->flags |= type;
                                //printf("%-12s: '%s' %s", "INV_GRIDRELAY", getDesc(devList[inv]->GridRelayStatus, "?"), ctime(&datetime));
                                break;

                            case BatChaStt:
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->BatChaStt = value;
                                devList[inv]->flags |= type;
                                break;

                            case BatDiagCapacThrpCnt:
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->BatDiagCapacThrpCnt = value;
                                devList[inv]->flags |= type;
                                break;

                            case BatDiagTotAhIn:
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->BatDiagTotAhIn = value;
                                devList[inv]->flags |= type;
                                break;

                            case BatDiagTotAhOut:
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->BatDiagTotAhOut = value;
                                devList[inv]->flags |= type;
                                break;

                            case BatTmpVal:
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->BatTmpVal = value;
                                devList[inv]->flags |= type;
                                break;

                            case BatVol:
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->BatVol = value;
                                devList[inv]->flags |= type;
                                break;

                            case BatAmp:
                                if (recordsize == 0) recordsize = 28;
                                devList[inv]->BatAmp = value;
                                devList[inv]->flags |= type;
                                break;

							case CoolsysTmpNom:
                                if (recordsize == 0) recordsize = 28;
								devList[inv]->Temperature = value;
								devList[inv]->flags |= type;
								break;

							case MeteringGridMsTotWhOut:
                                if (recordsize == 0) recordsize = 28;
								devList[inv]->MeteringGridMsTotWOut = value;
								break;

							case MeteringGridMsTotWhIn:
                                if (recordsize == 0) recordsize = 28;
								devList[inv]->MeteringGridMsTotWIn = value;
								break;

                            default:
                                if (recordsize == 0) recordsize = 12;
                            }
                    }
                }
                else
                {
                    printf("Packet ID mismatch. Expected %d, received %d\n", pcktID, rcvpcktID);
                }
            }
        }
        while (validPcktID == 0);
    }

    return E_OK;
}

void resetInverterData(InverterData *inv)
{
	inv->BatAmp = 0;
	inv->BatChaStt = 0;
	inv->BatDiagCapacThrpCnt = 0;
	inv->BatDiagTotAhIn = 0;
	inv->BatDiagTotAhOut = 0;
	inv->BatTmpVal = 0;
	inv->BatVol = 0;
	inv->BT_Signal = 0;
	inv->calEfficiency = 0;
	inv->calPacTot = 0;
	inv->calPdcTot = 0;
	inv->DeviceClass[0] = 0;
	inv->DeviceName[0] = 0;
	inv->DeviceStatus = 0;
	inv->DeviceType[0] = 0;
	inv->EToday = 0;
	inv->ETotal = 0;
	inv->FeedInTime = 0;
	inv->flags = 0;
	inv->GridFreq = 0;
	inv->GridRelayStatus = 0;
	inv->Iac1 = 0;
	inv->Iac2 = 0;
	inv->Iac3 = 0;
	inv->Idc1 = 0;
	inv->Idc2 = 0;
	inv->InverterDatetime = 0;
	inv->IPAddress[0] = 0;
	inv->modelID = 0;
	inv->NetID = 0;
	inv->OperationTime = 0;
	inv->Pac1 = 0;
	inv->Pac2 = 0;
	inv->Pac3 = 0;
	inv->Pdc1 = 0;
	inv->Pdc2 = 0;
	inv->Pmax1 = 0;
	inv->Pmax2 = 0;
	inv->Pmax3 = 0;
	inv->Serial = 0;
	inv->SleepTime = 0;
	inv->SUSyID = 0;
	inv->SWVersion[0] = 0;
	inv->Temperature = 0;
	inv->TotalPac = 0;
	inv->Uac1 = 0;
	inv->Uac2 = 0;
	inv->Uac3 = 0;
	inv->Udc1 = 0;
	inv->Udc2 = 0;
	inv->WakeupTime = 0;
	inv->monthDataOffset = 0;
	inv->multigateID = -1;
	inv->MeteringGridMsTotWIn = 0;
	inv->MeteringGridMsTotWOut = 0;
}

//Power Values are missing on some inverters
void CalcMissingSpot(InverterData *invData)
{
	if (invData->Pdc1 == 0) invData->Pdc1 = (invData->Idc1 * invData->Udc1) / 100000;
	if (invData->Pdc2 == 0) invData->Pdc2 = (invData->Idc2 * invData->Udc2) / 100000;

	if (invData->Pac1 == 0) invData->Pac1 = (invData->Iac1 * invData->Uac1) / 100000;
	if (invData->Pac2 == 0) invData->Pac2 = (invData->Iac2 * invData->Uac2) / 100000;
	if (invData->Pac3 == 0) invData->Pac3 = (invData->Iac3 * invData->Uac3) / 100000;

    if (invData->TotalPac == 0) invData->TotalPac = invData->Pac1 + invData->Pac2 + invData->Pac3;
}

E_SBFSPOT ethGetPacket(void)
{
    printf("ethGetPacket()\n");
    E_SBFSPOT rc = E_OK;

    ethPacketHeaderL1L2 *pkHdr = (ethPacketHeaderL1L2 *)CommBuf;

    do
    {
        int bib = ethRead(CommBuf, sizeof(CommBuf));

        if (bib <= 0)
        {
            printf("No data!\n");
            rc = E_NODATA;
        }
        else
        {
            unsigned short pkLen = (pkHdr->pcktHdrL1.hiPacketLen << 8) + pkHdr->pcktHdrL1.loPacketLen;

            //More data after header?
            if (pkLen > 0)
            {
	            HexDump(CommBuf, bib, 10);
                if (btohl(pkHdr->pcktHdrL2.MagicNumber) == ETH_L2SIGNATURE)
                {
                    // Copy CommBuf to packetbuffer
                    // Dummy byte to align with BTH (7E)
                    pcktBuf[0]= 0;
                    // We need last 6 bytes of ethPacketHeader too
                    memcpy(pcktBuf+1, CommBuf + sizeof(ethPacketHeaderL1), bib - sizeof(ethPacketHeaderL1));
                    // Point packetposition at last byte in our buffer
					// This is different from BTH
                    packetposition = bib - sizeof(ethPacketHeaderL1);

                    printf("<<<====== Content of pcktBuf =======>>>\n");
                    HexDump(pcktBuf, packetposition, 10);
                    printf("<<<=================================>>>\n");
                    
                    rc = E_OK;
                }
                else
                {
                    printf("L2 header not found.\n");
                    rc = E_RETRY;
                }
            }
            else
                rc = E_NODATA;
        }
    } while (rc == E_RETRY);

    return rc;
}


// Initialise multiple ethernet connected inverters
E_SBFSPOT ethInitConnectionMulti(InverterData *inverters[], char list[][16])
{
    puts("Initializing...");

    //Generate a Serial Number for application
    AppSUSyID = 125;
    srand(time(NULL));
    AppSerial = 900000000 + ((rand() << 16) + rand()) % 100000000;

	printf("SUSyID: %d - SessionID: %lu\n", AppSUSyID, AppSerial);

    E_SBFSPOT rc = E_OK;

    for (unsigned int devcount = 0; devcount < num_inverters; devcount++)
	{
		inverters[devcount] = malloc(sizeof(InverterData));
		resetInverterData(inverters[devcount]);
		strcpy(inverters[devcount]->IPAddress, ips[devcount]);
        printf("Inverter IP address: %s\n", inverters[devcount]->IPAddress);

		writePacketHeader(pcktBuf, 0, NULL);
		writePacket(pcktBuf, 0x09, 0xA0, 0, anySUSyID, anySerial);
		writeLong(pcktBuf, 0x00000200);
		writeLong(pcktBuf, 0);
		writeLong(pcktBuf, 0);
		writeLong(pcktBuf, 0);
		writePacketLength(pcktBuf);

		ethSend(pcktBuf, inverters[devcount]->IPAddress);

		if ((rc = ethGetPacket()) == E_OK)
		{
			ethPacket *pckt = (ethPacket *)pcktBuf;
			inverters[devcount]->SUSyID = btohs(pckt->Source.SUSyID);
			inverters[devcount]->Serial = btohl(pckt->Source.Serial);
			printf("Inverter replied: %s SUSyID: %d - Serial: %lu\n", inverters[devcount]->IPAddress, inverters[devcount]->SUSyID, inverters[devcount]->Serial);
		}
		else
		{
			printf("ERROR: Connection to inverter failed!\n");
			printf("Is %s the correct IP?\n", inverters[devcount]->IPAddress);
			return E_INIT;
		}

		logoffSMAInverter(inverters[devcount]);
	}

    return rc;
}

E_SBFSPOT logonSMAInverter(InverterData *inverters[], long userGroup, char *password)
{
#define MAX_PWLENGTH 12
    unsigned char pw[MAX_PWLENGTH] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    puts("logonSMAInverter()");

    char encChar = (userGroup == UG_USER)? 0x88:0xBB;
    //Encode password
    unsigned int idx;
    for (idx = 0; (password[idx] != 0) && (idx < sizeof(pw)); idx++)
        pw[idx] = password[idx] + encChar;
    for (; idx < MAX_PWLENGTH; idx++)
        pw[idx] = encChar;

    E_SBFSPOT rc = E_OK;
    int validPcktID = 0;

    time_t now;

	for (int inv=0; inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
	{
		printf("Logining into %s\n",inverters[inv]->IPAddress);
		do
		{
			pcktID++;
			now = time(NULL);
			writePacketHeader(pcktBuf, 0x01, addr_unknown);
			if (inverters[inv]->SUSyID != SID_SB240)
				writePacket(pcktBuf, 0x0E, 0xA0, 0x0100, inverters[inv]->SUSyID, inverters[inv]->Serial);
			else
				writePacket(pcktBuf, 0x0E, 0xE0, 0x0100, inverters[inv]->SUSyID, inverters[inv]->Serial);
			writeLong(pcktBuf, 0xFFFD040C);
			writeLong(pcktBuf, userGroup);	// User / Installer
			writeLong(pcktBuf, 0x00000384); // Timeout = 900sec ?
			writeLong(pcktBuf, now);
			writeLong(pcktBuf, 0);
			writeArray(pcktBuf, pw, sizeof(pw));
			writePacketTrailer(pcktBuf);
			writePacketLength(pcktBuf);
		}
		while (!1);

		ethSend(pcktBuf, inverters[inv]->IPAddress);
		validPcktID = 0;

		do
		{
			if ((rc = ethGetPacket()) == E_OK)
			{
				ethPacket *pckt = (ethPacket *)pcktBuf;
				if (pcktID == (btohs(pckt->PacketID) & 0x7FFF))   // Valid Packet ID
				{
					validPcktID = 1;
					unsigned short retcode = btohs(pckt->ErrorCode);
					switch (retcode)
					{
						case 0: rc = E_OK; break;
						case 0x0100: rc = E_INVPASSW; break;
						default: rc = E_LOGONFAILED; break;
					}
				}
				else
					printf("Packet ID mismatch. Expected %d, received %d\n", pcktID, (btohs(pckt->PacketID) & 0x7FFF));
			}
		} while ((validPcktID == 0) && (rc == E_OK)); // Fix Issue 167
	}
    return rc;
}

E_SBFSPOT logoffSMAInverter(InverterData *inverter)
{
    puts("logoffSMAInverter()");
    do
    {
        pcktID++;
        writePacketHeader(pcktBuf, 0x01, addr_unknown);
        writePacket(pcktBuf, 0x08, 0xA0, 0x0300, anySUSyID, anySerial);
        writeLong(pcktBuf, 0xFFFD010E);
        writeLong(pcktBuf, 0xFFFFFFFF);
        writePacketTrailer(pcktBuf);
        writePacketLength(pcktBuf);
    }
    while (!1);

    ethSend(pcktBuf, inverter->IPAddress);

    return E_OK;
}


E_SBFSPOT logoffMultigateDevices(InverterData *inverters[])
{
    puts("logoffMultigateDevices()");
	for (int mg=0; inverters[mg]!=NULL && mg<MAX_INVERTERS; mg++)
	{
		InverterData *pmg = inverters[mg];
		if (pmg->SUSyID == SID_MULTIGATE)
		{
			pmg->hasDayData = 1;//true;
			for (int sb240=0; inverters[sb240]!=NULL && sb240<MAX_INVERTERS; sb240++)
			{
				InverterData *psb = inverters[sb240];
				if ((psb->SUSyID == SID_SB240) && (psb->multigateID == mg))
				{		
					//do
					//{
						pcktID++;
						writePacketHeader(pcktBuf, 0, NULL);
						writePacket(pcktBuf, 0x08, 0xE0, 0x0300, psb->SUSyID, psb->Serial);
						writeLong(pcktBuf, 0xFFFD010E);
						writeLong(pcktBuf, 0xFFFFFFFF);
						writePacketTrailer(pcktBuf);
						writePacketLength(pcktBuf);
					//}
					//while (!isCrcValid(pcktBuf[packetposition-3], pcktBuf[packetposition-2]));

					ethSend(pcktBuf, psb->IPAddress);

					psb->logonStatus = 0; // logged of

					printf("Logoff %d:%d\n",psb->SUSyID, psb->Serial);
				}
			}
		}
	}

    return E_OK;
}

// Ethernet.c //
const char *IP_Broadcast = "239.12.255.254";

int ethConnect(short port)
{
    int ret = 0;
    // create socket for UDP
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    {
        printf ("Socket error : %i\n", sock);
        return -1;
    }

    // set up parameters for UDP listen socket
    memset((char *)&addr_out, 0, sizeof(addr_out));
    addr_out.sin_family = AF_INET;
    addr_out.sin_port = htons(port);
    addr_out.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// bind this port (on any interface) to our UDP listener
    ret = bind(sock, (struct sockaddr*) &addr_out, sizeof(addr_out));
    // here is the destination IP
	addr_out.sin_addr.s_addr = inet_addr(IP_Broadcast);

    // Set options to receive broadcast packets
    struct ip_mreq mreq;
    
    mreq.imr_multiaddr.s_addr = inet_addr(IP_Broadcast);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq));
	unsigned char loop = 0;
    ret = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (const char *)&loop, sizeof(loop));

    if (ret < 0)
    {
        printf ("setsockopt IP_ADD_MEMBERSHIP failed\n");
        return -1;
    }
    // end of setting broadcast options

    return 0; //OK
}

int ethRead(unsigned char *buf, unsigned int bufsize)
{
    int bytes_read;
    short timeout = 5;
    socklen_t addr_in_len = sizeof(addr_in);

    fd_set readfds;

	do
	{
		struct timeval tv;
		tv.tv_sec = timeout;     //set timeout of reading
		tv.tv_usec = 0;

		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);

		int rc = select(sock+1, &readfds, NULL, NULL, &tv);
		printf("select() returned %d\n", rc);
		if (rc == -1)
		{
			printf("errno = %d\n", errno);
		}

		if (FD_ISSET(sock, &readfds))
			bytes_read = recvfrom(sock, (char*)buf, bufsize, 0, (struct sockaddr *)&addr_in, &addr_in_len);
		else
		{
			puts("Timeout reading socket");
			return -1;
		}

		if ( bytes_read > 0)
		{
			if (bytes_read > MAX_CommBuf)
			{
				MAX_CommBuf = bytes_read;
				printf("MAX_CommBuf is now %d bytes\n", MAX_CommBuf);
			}
			printf("Received %d bytes from IP [%s]\n", bytes_read, inet_ntoa(addr_in.sin_addr));
			if (bytes_read == 600 || bytes_read == 608 || bytes_read == 0)
		   		printf(" ==> packet ignored\n");
		}
		else
			printf("recvfrom() returned an error: %d\n", bytes_read);

	} while (bytes_read == 600 || bytes_read == 608); // keep on reading if data received from Energy Meter (600 bytes) or Sunny Home Manager (608 bytes)

    return bytes_read;
}

int ethSend(unsigned char *buffer, const char *toIP)
{
	HexDump(buffer, packetposition, 10);

	addr_out.sin_addr.s_addr = inet_addr(toIP);
    size_t bytes_sent = sendto(sock, (const char*)buffer, packetposition, 0, (struct sockaddr *)&addr_out, sizeof(addr_out));

	printf(" %d Bytes sent to IP [%s]\n", bytes_sent, inet_ntoa(addr_out.sin_addr));
    return bytes_sent;
}


int ethClose()
{
	if (sock != 0)
	{
		close(sock);
		sock = 0;
	}
    return 0;
}

int getLocalIP(unsigned char IPaddress[4])
{
    int rc = 0;
    
    struct ifaddrs *myaddrs;
    struct in_addr *inaddr;

    if(getifaddrs(&myaddrs) == 0)
    {
        for (struct ifaddrs *ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr != NULL)
            {
                // Find the active network adapter
                if ((ifa->ifa_addr->sa_family == AF_INET) && (ifa->ifa_flags & IFF_UP) && (strcmp(ifa->ifa_name, "lo") != 0))
                {
                    struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                    inaddr = &s4->sin_addr;

                    unsigned long ipaddr = inaddr->s_addr;
                    IPaddress[3] = ipaddr & 0xFF;
                    IPaddress[2] = (ipaddr >> 8) & 0xFF;
                    IPaddress[1] = (ipaddr >> 16) & 0xFF;
                    IPaddress[0] = (ipaddr >> 24) & 0xFF;

                    break;
                }
            }
        }

        freeifaddrs(myaddrs);
    }
    else
        rc = -1;
    
    return rc;
}

void writeLong(BYTE *btbuffer, unsigned long v)
{
    writeByte(btbuffer,(unsigned char)((v >> 0) & 0xFF));
    writeByte(btbuffer,(unsigned char)((v >> 8) & 0xFF));
    writeByte(btbuffer,(unsigned char)((v >> 16) & 0xFF));
    writeByte(btbuffer,(unsigned char)((v >> 24) & 0xFF));
}

void writeShort(BYTE *btbuffer, unsigned short v)
{
    writeByte(btbuffer,(unsigned char)((v >> 0) & 0xFF));
    writeByte(btbuffer,(unsigned char)((v >> 8) & 0xFF));
}

void writeByte(unsigned char *btbuffer, unsigned char v)
{
    btbuffer[packetposition++] = v;
}

void writeArray(unsigned char *btbuffer, const unsigned char bytes[], int loopcount)
{
    for (int i = 0; i < loopcount; i++)
    {
        writeByte(btbuffer, bytes[i]);
    }
}

void writePacket(unsigned char *buf, unsigned char longwords, unsigned char ctrl, unsigned short ctrl2, unsigned short dstSUSyID, unsigned long dstSerial)
{
	writeLong(buf, ETH_L2SIGNATURE);

    writeByte(buf, longwords);
    writeByte(buf, ctrl);
    writeShort(buf, dstSUSyID);
    writeLong(buf, dstSerial);
    writeShort(buf, ctrl2);
    writeShort(buf, AppSUSyID);
    writeLong(buf, AppSerial);
    writeShort(buf, ctrl2);
    writeShort(buf, 0);
    writeShort(buf, 0);
    writeShort(buf, pcktID | 0x8000);
}

void writePacketTrailer(unsigned char *btbuffer)
{
   	writeLong(btbuffer, 0);
}

void writePacketHeader(unsigned char *buf, const unsigned int control, const unsigned char *destaddress)
{
	packetposition = 0;

    //Ignore control and destaddress
    writeLong(buf, 0x00414D53);  // SMA\0
    writeLong(buf, 0xA0020400);
    writeLong(buf, 0x01000000);
    writeByte(buf, 0);
    writeByte(buf, 0);          // Placeholder for packet length
}

void writePacketLength(unsigned char *buf)
{
    short dataLength = (short)(packetposition - sizeof(ethPacketHeaderL1L2));
    ethPacketHeaderL1L2 *hdr = (ethPacketHeaderL1L2 *)buf;
    hdr->pcktHdrL1.hiPacketLen = (dataLength >> 8) & 0xFF;
    hdr->pcktHdrL1.loPacketLen = dataLength & 0xFF;
}

int64_t get_longlong(BYTE *buf)
{
    register int64_t lnglng = 0;

	lnglng += *(buf+7);
	lnglng <<= 8;
	lnglng += *(buf+6);
	lnglng <<= 8;
	lnglng += *(buf+5);
	lnglng <<= 8;
	lnglng += *(buf+4);
	lnglng <<= 8;
	lnglng += *(buf+3);
	lnglng <<= 8;
	lnglng += *(buf+2);
	lnglng <<= 8;
	lnglng += *(buf+1);
	lnglng <<= 8;
	lnglng += *(buf);

    return lnglng;
}

int32_t get_long(BYTE *buf)
{
    register int32_t lng = 0;

	lng += *(buf+3);
	lng <<= 8;
	lng += *(buf+2);
	lng <<= 8;
	lng += *(buf+1);
	lng <<= 8;
	lng += *(buf);

    return lng;
}

short get_short(BYTE *buf)
{
    register short shrt = 0;

	shrt += *(buf+1);
	shrt <<= 8;
	shrt += *(buf);

    return shrt;
}

int getInverterIndexBySerial(InverterData *inverters[], unsigned short SUSyID, uint32_t Serial)
{
	printf("getInverterIndexBySerial()\n");
	printf("Looking up %d:%lu\n", SUSyID, (unsigned long)Serial);

    for (int inv=0; inverters[inv]!=NULL && inv<MAX_INVERTERS; inv++)
    {
		printf("Inverter[%d] %d:%lu\n", inv, inverters[inv]->SUSyID, inverters[inv]->Serial);

		if ((inverters[inv]->SUSyID == SUSyID) && inverters[inv]->Serial == Serial)
            return inv;
    }

	printf("Serial Not Found!\n");
	
	return -1;
}
>>>>>>> 12f757d22287fb02657da2ee1e19dcd0329c51c1
