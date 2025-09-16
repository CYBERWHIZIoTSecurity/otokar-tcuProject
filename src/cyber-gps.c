#include "include/cyber-gps.h"

int doGsmActions()
{
	int ret = 0;
	ret = check_gsm_modem_status();
	if (ret != 0)
	{
		ret = gsm_modem_on("0000", 4);
		if (ret != 0)
		{
			printf("GSM modem on failed, ret=0x%x\n", ret);
			goto fail;
		}
		printf("GSM modem status check failed and modem on!");
		goto success;
	}
	printf("GSM modem status check success\n");
	goto success;

fail:
	ret = -1;
success:
	return ret;
}

int init_gps()
{
	int ret = 0;
	ret = gps_init();
	if (ret != 0)
	{
		printf("GPS init failed, ret=0x%x\n", ret);
		goto fail;
	}
	printf("GPS init success\n");
	goto success;

fail:
	ret = -1;
success:
	return ret;
}

int deinit_gps()
{
	int ret = 0;
	ret = gps_deinit();
	if (ret != 0)
	{
		printf("GPS deinit failed, ret=0x%x\n", ret);
		goto fail;
	}
	printf("GPS deinit success\n");
	goto success;

fail:
	ret = -1;
success:
	return ret;
}

int read_gps_data()
{
	int ret = 0;
	char recv_data[200] = { 0 };
	size_t len = 0;

	sleep(1);
	ret = get_gps_data("GPRMC", &len, recv_data, sizeof(recv_data));
	if (ret != 0)
	{
		printf("Get GPS data failed, ret=0x%x\n", ret);
		goto fail;
	}
	printf("data=%s\n", recv_data);
	goto success;

fail:
	ret = -1;
success:
	return ret;
}

int main()
{
	int ret = 0;

	ret = doGsmActions();
	if (ret != 0)
	{
		return -1;
	}

	ret = init_gps();
	if (ret != 0)
	{
		return -1;
	}

	while(1)
	{
		ret = read_gps_data();
		if (ret != 0)
		{
			deinit_gps();
			return -1;
		}
		sleep(1);
	}

	ret = deinit_gps();
	if (ret != 0)
	{
		return -1;
	}

	return 0;
}