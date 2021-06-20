#include <libsmu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>

enum rsmu_op {
	TEST_MESSAGE = 0x01, // arg0 += 1
	GET_SMU_VERSION = 0x02, // res0
	// matisse & vermeer commands
	TRANSFER_TABLE_SMU2DRAM = 0x05,
	GET_DRAM_BASE_ADDRESS = 0x06, // res0 = phyaddr
	GET_PM_TABLE_VERSION = 0x08, // res0 = version
	SET_VDDCR_SOC = 0x14, // arg0 = vid
	SET_PPT_LIMIT = 0x53, // arg0 = mW
	SET_TDC_LIMIT = 0x54, // arg0 = mA
	SET_EDC_LIMIT = 0x55, // arg0 = mA
	SET_CHTC_LIMIT = 0x56, // arg0 = C
	SET_PBO_SCALAR = 0x58, // arg0 = 1000 - (scalar * 100)
	GET_FASTEST_CORE_OF_SOCKET = 0x59, // = 16 * BYTES2(res0)
	SET_PROCHOT_STATUS = 0x5a, // arg0 = status, Disabled(0x1000000)
	ENABLE_OVERCLOCKING = 0x5a, // arg0 = status, Enabled(0)
	DISABLE_OVERCLOCKING = 0x5b, // arg0 = status, Disabled(0x1000000)
	SET_OVERCLOCK_FREQ_ALL_CORES = 0x5c, // arg0 = frequency
	SET_OVERCLOCK_FREQ_PER_CORE = 0x5d, // arg0 = value (TODO)
	SET_OVERCLOCK_CPU_VID = 0x61, // arg0 = vid,
	GET_PBO_SCALAR = 0x6c, // res0 = scalar from 1~10
	GET_MAX_FREQUENCY = 0x6e, // res0 = MHz
	GET_PROCESSOR_PARAMETERS = 0x6f, // res0 = bitfield, bit0 = isoverclockable, bit1 = fusedMOCstatus/pbo support
};

enum flag {
	SMU_ENABLED = 0,
	SMU_DISABLED = 0x1000000,
};

#define FREQ_MAX 8000

int set_overclock_freq_for_core(smu_arg_t* args, uint8_t core, uint8_t ccx, uint8_t ccd, uint16_t frequency) {
	if (frequency > FREQ_MAX || core > 3 || ccx > 1 || ccd > 1) {
		return -1;
	}

	uint16_t core_specifier = core | (ccx << 4) | (ccd << 8);
	args->i.args0 = (uint32_t)frequency | ((uint32_t)core_specifier << 20);

	return 0;
}

void help() {
	printf("help me\n");
}

#define TODO do { printf("TODO\n"); return 1; } while (0)

int main(int argc, const char* argv[]) {
	enum smu_mailbox mailbox = TYPE_RSMU; // see also TYPE_MP1
	int op = GET_SMU_VERSION;

	smu_arg_t args;
	memset(&args, 0, sizeof(args));

	if (argc > 1) {
		const char* cmd = argv[1];
		if (!strcmp("rsmu", cmd)) {
			// custom rsmu command
			TODO;
		} else if (!strcmp("ppt", cmd)) {
			if (argc <= 2) {
				help();
				return 1;
			} else {
				int ppt = atoi(argv[2]);
				op = SET_PPT_LIMIT;
				args.i.args0 = ppt;
			}
		} else if (!strcmp("tdc", cmd)) {
			if (argc <= 2) {
				help();
				return 1;
			} else {
				int tdc = atoi(argv[2]);
				op = SET_TDC_LIMIT;
				args.i.args0 = tdc;
			}
		} else if (!strcmp("edc", cmd)) {
			if (argc <= 2) {
				help();
				return 1;
			} else {
				int edc = atoi(argv[2]);
				op = SET_EDC_LIMIT;
				args.i.args0 = edc;
			}
		} else if (!strcmp("thm", cmd)) {
			if (argc <= 2) {
				help();
				return 1;
			} else {
				int thm = atoi(argv[2]);
				op = SET_CHTC_LIMIT;
				args.i.args0 = thm;
			}
		} else if (!strcmp("scalar", cmd)) {
			if (argc <= 2) {
				op = GET_PBO_SCALAR;
			} else {
				int scalar = atoi(argv[2]);
				if (scalar < 1 || scalar > 10) {
					// why am I even writing C anyway wtf...
					printf("scalar out of range, expected 1~10\n");
					return 1;
				}
				op = SET_PBO_SCALAR;
				args.i.args0 = 1000 - scalar * 100;
			}
		} else if (!strcmp("vid", cmd)) {
			if (argc <= 2) {
				help();
				return 1;
			} else {
				int vid = atoi(argv[2]);
				if (vid <= 0) {
					// 0 = 1.55V so whatever, refuse to set it sorry
					printf("invalid VID\n");
					return 1;
				} else {
					uint32_t uv = 1550000 - vid * 6250;
					printf("setting VID to %f mV", (float)uv / 1000);
				}
				op = SET_OVERCLOCK_CPU_VID;
				args.i.args0 = vid;
			}
		} else if (!strcmp("freq", cmd)) {
			if (argc <= 2) {
				op = GET_MAX_FREQUENCY;
			} else {
				int freq = atoi(argv[2]);
				if (freq < 100 || freq > FREQ_MAX) {
					printf("freq out of range\n");
					return 1;
				}
				op = SET_OVERCLOCK_FREQ_ALL_CORES;
				args.i.args0 = freq;
			}
		} else if (!strcmp("corefreq", cmd)) {
			if (argc <= 5) {
				help();
				return 1;
			} else {
				int core = atoi(argv[2]);
				int ccx = atoi(argv[3]);
				int ccd = atoi(argv[4]);
				int freq = atoi(argv[5]);
				if (freq <= 0 || set_overclock_freq_for_core(&args, core, ccx, ccd, freq)) {
					printf("args out of range\n");
					return 1;
				}
				op = SET_OVERCLOCK_FREQ_PER_CORE;
			}
		} else if (!strcmp("oc", cmd)) {
			if (argc <= 2) {
				help();
				return 1;
			} else {
				const char* subcmd = argv[2];
				if (!strcmp("on", subcmd)) {
					op = ENABLE_OVERCLOCKING;
					args.i.args0 = SMU_ENABLED;
				} else if (!strcmp("off", subcmd)) {
					op = DISABLE_OVERCLOCKING;
					args.i.args0 = SMU_DISABLED;
				} else {
					printf("unknown subcommand %s\n", subcmd);
					return 1;
				}
			}
			// on, off
			TODO;
		} else {
			printf("unknown command %s\n", cmd);
			return 1;
		}
	}

	smu_obj_t smu;
	int res = smu_init(&smu);
	if (res != SMU_Return_OK) {
		printf("SMU init failed %d: %s\n", res, smu_return_to_str(res));
		return 1;
	}

	int exitcode = 0;

	const char* version_string = smu_get_fw_version(&smu);
	printf("SMU %s\n", version_string);

	res = smu_send_command(&smu, op, &args, mailbox);
	if (res != SMU_Return_OK) {
		printf("SMU command failed %d: %s\n", res, smu_return_to_str(res));
		exitcode = 1;
	} else {
		printf("SMU returned: %08x\n", args.i.args0);
	}

	smu_free(&smu);

	return exitcode;
}
