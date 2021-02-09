/*============================================================================

  Copyright (c) 2021 99degree
  licensed MIT

============================================================================*/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include "sensor_lib.h"
#include "eeprom_lib.h"
//#include "camera_config.h"
//#include "sensor_sdk_utils.h"
#include "media/msm_camsensor_sdk.h"

#undef DEBUG_INFO
#include "log.h"

#define  LOG_TAG    "camera_sensor_wrapper"
#define  ALOG(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#ifdef _LIB_
#define DEBUG_INFO ALOG
#else
#define DEBUG_INFO printf
#endif

struct camera_sensor_slave_info_mod {
  char sensor_name[32];
  unsigned short slave_addr;
  enum camera_i2c_freq_mode i2c_freq_mode;
  enum camera_i2c_reg_addr_type addr_type;
  enum camera_i2c_data_type data_type;
  struct sensor_id_info_t sensor_id_info;
  char dummy[0x9e10 - 0x4008 - 4 /* -4 from dump */ - sizeof(struct sensor_id_info_t) - sizeof(enum camera_i2c_data_type) \
  - sizeof(enum camera_i2c_reg_addr_type) - sizeof(enum camera_i2c_freq_mode) - sizeof(unsigned short) - sizeof(char [32])];
  struct camera_power_setting_array power_setting_array;
  unsigned char is_init_params_valid;
  struct camera_alt_slave_info sensor_alt_slave_info;
};

typedef struct {
	void         *sensor_lib_handle;
	void         *module_lib_handle;
	sensor_lib_t *sensor_lib_ptr;
	eeprom_lib_func_t *eeprom_lib_ptr;
//	struct msm_camera_sensor_slave_info sensor_slave_info_k;
//	struct msm_sensor_power_setting power_down_setting_k;
//	camera_module_config_t camera_cfg;
//	struct msm_sensor_power_setting power_up_setting_k;
//	struct camera_sensor_slave_info slave_info_u;
} sensor_lib_params_t;

struct camera_power_setting_array power_setting_array_imx362 =
    {
      .power_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 24000000,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_HIGH,
          .delay = 18,
        },
      },
      .size = 8,
      .power_down_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 0,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
      },
      .size_down = 7,
    };
	
struct camera_power_setting_array power_setting_array_s5k3p3sm =
    {
      .power_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 24000000,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
      },
      .size = 8,
      .power_down_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 0,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
      },
      .size_down = 7,
    };

static sensor_lib_params_t                  sensor_lib_params_0;

#ifndef SERR
#define SLOW DEBUG_INFO
#define SERR DEBUG_INFO
#endif

#define BUFF_SIZE_255 255
#define SENSOR_SUCCESS 0
#define SENSOR_FAILURE -1
#define SENSOR_ERROR_IO -2
#define SENSOR_ERROR_NOMEM -3
#define SENSOR_ERROR_INVAL -4

int32_t sensor_load_library(const char *name, void *data)
{
	char lib_name[BUFF_SIZE_255] = {0};
	char open_lib_str[BUFF_SIZE_255] = {0};
	void *(*sensor_open_lib_l)(void) = NULL;
	void *handle;
	sensor_lib_params_t *sensor_lib_params = (sensor_lib_params_t *)data;
	SLOW("enter\n");
	#ifndef _LIB_  
	snprintf(lib_name, BUFF_SIZE_255, "./libmmcamera_%s.so", name);
	#else
	snprintf(lib_name, BUFF_SIZE_255, "./libmmcamera_%s_1.so", name);
	#endif
	SLOW("lib_name %s\n", lib_name);
	sensor_lib_params->sensor_lib_handle = handle = dlopen(lib_name, RTLD_NOW); //RTLD_NOW
	if (!sensor_lib_params->sensor_lib_handle && !handle) {
		SERR("failed1\n");
		return -EINVAL;
	}

	snprintf(open_lib_str, BUFF_SIZE_255, "%s_open_lib", name);
	*(void **)&sensor_open_lib_l = dlsym(sensor_lib_params->sensor_lib_handle,
	open_lib_str);
	if (!sensor_open_lib_l) {
	*(void **)&sensor_open_lib_l = dlsym(sensor_lib_params->sensor_lib_handle,
		"sensor_open_lib");
	if (!sensor_open_lib_l) {
		SERR("failed1\n");
		return -EINVAL;
	}
	}
	sensor_lib_params->sensor_lib_ptr = (sensor_lib_t *)sensor_open_lib_l();
	if (!sensor_lib_params->sensor_lib_ptr) {
	SERR("failed2\n");
	return -EINVAL;
	}

	SLOW("exit\n");
	return 0;
}

int32_t eeprom_load_library(const char *name, void *data)
{
	char lib_name[BUFF_SIZE_255] = {0};
	char open_lib_str[BUFF_SIZE_255] = {0};
	void *(*sensor_open_lib_l)(void) = NULL;
	void *handle;
	sensor_lib_params_t *sensor_lib_params = (sensor_lib_params_t *)data;
	SLOW("enter\n");
	#ifndef _LIB_
	snprintf(lib_name, BUFF_SIZE_255, "./libmmcamera_%s_eeprom.so", name);
	#else
	snprintf(lib_name, BUFF_SIZE_255, "./libmmcamera_%s_eeprom_1.so", name);
	#endif
	SLOW("lib_name %s\n", lib_name);	

	sensor_lib_params->sensor_lib_handle = handle = dlopen(lib_name, RTLD_NOW);
	if (!sensor_lib_params->sensor_lib_handle && !handle) {
		DEBUG_INFO("%s\n", dlerror());
		SERR("failed00 %s\n", lib_name);
		return -EINVAL;
	}

	snprintf(open_lib_str, BUFF_SIZE_255, "%s_eeprom_open_lib", name);
	*(void **)&sensor_open_lib_l = dlsym(sensor_lib_params->sensor_lib_handle,
	open_lib_str);
	if (!sensor_open_lib_l) {
		SERR("failed3\n");
		return -EINVAL;
	}

	sensor_lib_params->eeprom_lib_ptr = (eeprom_lib_func_t *)sensor_open_lib_l();
	if (!sensor_lib_params->eeprom_lib_ptr) {
	SERR("failed4\n");
	return -EINVAL;
	}

	SLOW("exit\n");
	return 0;
}
#if 0
int32_t module_load_library(const char *name, void *data)
{
	char lib_name[BUFF_SIZE_255] = {0};
	char open_lib_str[BUFF_SIZE_255] = {0};

	void (*sensor_open_lib_l)(
	struct msm_camera_sensor_slave_info* ,
	struct camera_sensor_slave_info* ,
	camera_module_config_t* ,
	struct msm_sensor_power_setting* ,
	struct msm_sensor_power_setting* ) = NULL;

	void *handle;
	sensor_lib_params_t *sensor_lib_params = (sensor_lib_params_t *)data;
	SLOW("enter\n");

	snprintf(lib_name, BUFF_SIZE_255, "libmmcamera2_sensor_modules.so", name);

	SLOW("lib_name %s\n", lib_name);	
	
	sensor_lib_params->module_lib_handle = handle = dlopen(lib_name, RTLD_NOW);
	if (!sensor_lib_params->module_lib_handle && !handle) {
		DEBUG_INFO("%s\n", dlerror());
		SERR("failed00a %s\n", lib_name);
		return -EINVAL;
	}
	
	snprintf(open_lib_str, BUFF_SIZE_255, "translate_sensor_slave_info", name);
	*(void **)&sensor_open_lib_l = dlsym(sensor_lib_params->module_lib_handle,
	open_lib_str);
	if (!sensor_open_lib_l) {
		SERR("failed3\n");
		return -EINVAL;
	}

	sensor_open_lib_l(&sensor_lib_params->sensor_slave_info_k,
					&sensor_lib_params->sensor_lib_ptr->sensor_slave_info,
					&sensor_lib_params->camera_cfg,
					&sensor_lib_params->power_up_setting_k,
					&sensor_lib_params->power_down_setting_k);

	SLOW("exit\n");
	return 0;
}
#endif

int32_t sensor_unload_library(sensor_lib_params_t *sensor_lib_params)
{
	if (!sensor_lib_params) {
		SERR("failed7\n");
		return SENSOR_FAILURE;
	}
	if (sensor_lib_params->sensor_lib_handle) {
		dlclose(sensor_lib_params->sensor_lib_handle);
		sensor_lib_params->sensor_lib_handle = NULL;
		sensor_lib_params->sensor_lib_ptr = NULL;
	}
  
    if (sensor_lib_params->module_lib_handle) {
		dlclose(sensor_lib_params->module_lib_handle);
	}
    return SENSOR_SUCCESS;
}

void __attach(void) __attribute__((constructor));
void __detach(void) __attribute__((destructor));

void *imx350_open_lib(void)
{
	static char *sensor = "imx350\0";

	if (sensor_load_library(sensor, &sensor_lib_params_0))
		return NULL;

	if (sensor_lib_params_0.sensor_lib_ptr->sensor_slave_info.power_setting_array.size == 0)
	{
		sensor_lib_params_0.sensor_lib_ptr->sensor_slave_info.power_setting_array = power_setting_array_imx362;
	}
	return sensor_lib_params_0.sensor_lib_ptr;
}

void *imx362_open_lib(void)
{
	static char *sensor = "imx362\0";

	if (sensor_load_library(sensor, &sensor_lib_params_0))
		return NULL;

	if (sensor_lib_params_0.sensor_lib_ptr->sensor_slave_info.power_setting_array.size == 0)
	{
		sensor_lib_params_0.sensor_lib_ptr->sensor_slave_info.power_setting_array = power_setting_array_imx362;
	}
	return sensor_lib_params_0.sensor_lib_ptr;
}

void *s5k4h7yx_open_lib(void)
{
	static char *sensor = "s5k4h7yx\0";

	if (sensor_load_library(sensor, &sensor_lib_params_0))
		return NULL;

	if (sensor_lib_params_0.sensor_lib_ptr->sensor_slave_info.power_setting_array.size == 0)
	{
		sensor_lib_params_0.sensor_lib_ptr->sensor_slave_info.power_setting_array = power_setting_array_s5k3p3sm;
	}
	return sensor_lib_params_0.sensor_lib_ptr;
}

void *sensor_open_lib(void)
{
#ifdef _IMX350_
    return imx350_open_lib();
#elif _IMX362_
	return imx362_open_lib();
#elif _S5K4H7YX_
	return s5k4h7yx_open_lib();
#endif
}

void *sony_imx350_eeprom_open_lib(void)
{
        static char *sensor = "sony_imx350\0";

		DEBUG_INFO("sony_imx350_eeprom_open_lib\n");
        if (eeprom_load_library(sensor, &sensor_lib_params_0))
                return NULL;

        if (sensor_lib_params_0.eeprom_lib_ptr->eeprom_info.power_setting_array.size == 1)
        {
                sensor_lib_params_0.eeprom_lib_ptr->eeprom_info.power_setting_array = power_setting_array_imx362;
        }
        return sensor_lib_params_0.eeprom_lib_ptr;
}

void sony_imx350_eeprom_get_calibration_items(void *e_ctrl)
{
	  char lib_name[BUFF_SIZE_255] = {0};
  char open_lib_str[BUFF_SIZE_255] = {0};
  void (*sensor_open_lib_l)(void*) = NULL;
  char * name = "sony_imx350_eeprom\0";
  sensor_lib_params_t *sensor_lib_params = &sensor_lib_params_0;
  
	snprintf(open_lib_str, BUFF_SIZE_255, "%s_get_calibration_items", name);
  sensor_open_lib_l = dlsym(sensor_lib_params->sensor_lib_handle,
    open_lib_str);
  if (!sensor_open_lib_l) {
        SERR("failed1\n");
        return;
    }
  sensor_open_lib_l(e_ctrl);
}
static void sony_imx350_eeprom_format_lscdata(
        sensor_eeprom_data_t *e_ctrl)
{
		  char lib_name[BUFF_SIZE_255] = {0};
  char open_lib_str[BUFF_SIZE_255] = {0};
  void (*sensor_open_lib_l)(void*) = NULL;
  sensor_lib_params_t *sensor_lib_params = &sensor_lib_params_0;
  char * name = "sony_imx350_eeprom\0";
  
	snprintf(open_lib_str, BUFF_SIZE_255, "%s_format_lscdata", name);
  sensor_open_lib_l = dlsym(sensor_lib_params->sensor_lib_handle,
    open_lib_str);
  if (!sensor_open_lib_l) {
        SERR("failed1\n");
        return;
    }
  sensor_open_lib_l(e_ctrl);
}
void sony_imx350_eeprom_format_calibration_data(void *e_ctrl)
{
		  char lib_name[BUFF_SIZE_255] = {0};
  char open_lib_str[BUFF_SIZE_255] = {0};
  void (*sensor_open_lib_l)(void*) = NULL;
  char * name = "sony_imx350_eeprom\0";
	sensor_lib_params_t *sensor_lib_params = &sensor_lib_params_0;
	
	snprintf(open_lib_str, BUFF_SIZE_255, "%s_format_calibration_data", name);
  sensor_open_lib_l = dlsym(sensor_lib_params->sensor_lib_handle,
    open_lib_str);
  if (!sensor_open_lib_l) {
        SERR("failed1\n");
        return;
    }
  sensor_open_lib_l(e_ctrl);
}


void __attach(void)
{
	DEBUG_INFO("__attach()\n");
}

void __detach(void)
{
	DEBUG_INFO("__detach()\n");
	sensor_unload_library(&sensor_lib_params_0);
}

/*
	below hexDump function is copyrighted to paxdiablo from stackoverflow
	https://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
*/
void hexDump(char *desc, void *addr, int len) 
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                DEBUG_INFO("  %s\n", buff);

            // Output the offset.
            DEBUG_INFO("  %04x ", i);
        }

        // Now the hex code for the specific character.
        DEBUG_INFO(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        DEBUG_INFO("   ");
        i++;
    }

    // And print the final ASCII bit.
    DEBUG_INFO("  %s\n", buff);
}

void print_memory_map_array(struct eeprom_memory_map_array *mem_map_array)
{
	int i,j;
	
	if(mem_map_array->size_map_array)
	{
		DEBUG_INFO(".size_map_array = %d,\n", mem_map_array->size_map_array);
		for(i=0;i<mem_map_array->size_map_array;i++)
		{
			DEBUG_INFO(".memory_map = {\n");
			DEBUG_INFO("	.slave_addr = 0x%x,\n", mem_map_array->memory_map[i].slave_addr);
			DEBUG_INFO("	.memory_map_size = %d,\n", mem_map_array->memory_map[i].memory_map_size);
			DEBUG_INFO("	.mem_settings = {\n");
#if 1
			for(j=0;j<mem_map_array->memory_map[i].memory_map_size;j++) {
				DEBUG_INFO("     	{\n");
				DEBUG_INFO("		.reg_addr = 0x%x,\n",  mem_map_array->memory_map[i].mem_settings[j].reg_addr);
				DEBUG_INFO("		.addr_type = %d,\n",  mem_map_array->memory_map[i].mem_settings[j].addr_type);
				DEBUG_INFO("		.reg_data = %d,\n",  mem_map_array->memory_map[i].mem_settings[j].reg_data);
				DEBUG_INFO("		.data_type = %d,\n",  mem_map_array->memory_map[i].mem_settings[j].data_type);
				DEBUG_INFO("		.i2c_operation = %d,\n",  mem_map_array->memory_map[i].mem_settings[j].i2c_operation);
				DEBUG_INFO("		.delay = %d,\n",  mem_map_array->memory_map[i].mem_settings[j].delay);				
				DEBUG_INFO("     	},\n");
			}
#else
			for(j=0;j<mem_map_array->memory_map[i].memory_map_size;j++) {
				DEBUG_INFO("     	{0x%x, 0x%x, 0x%x},\n",mem_map_array->memory_map[i].mem_settings[j].reg_addr, mem_map_array->memory_map[i].mem_settings[j].reg_data, mem_map_array->memory_map[i].mem_settings[j].data_type);
			}	
			
#endif
			DEBUG_INFO("},\n");
		}
	} else {
		DEBUG_INFO("memory_map not found!\n");
	}
};

static char * camera_power_seq_type_str[10];
static char* camera_vreg_name_str[8];

void print_power_setting_array(struct camera_power_setting_array *power_setting_array)
{
	int i;
		
	camera_power_seq_type_str[CAMERA_POW_SEQ_CLK] = "CAMERA_POW_SEQ_CLK";
	camera_power_seq_type_str[CAMERA_POW_SEQ_GPIO] = "CAMERA_POW_SEQ_GPIO";
	camera_power_seq_type_str[CAMERA_POW_SEQ_VREG]="CAMERA_POW_SEQ_VREG";
	camera_power_seq_type_str[CAMERA_POW_SEQ_I2C_MUX]="CAMERA_POW_SEQ_I2C_MUX";
	camera_power_seq_type_str[CAMERA_POW_SEQ_I2C]="CAMERA_POW_SEQ_I2C";

	camera_vreg_name_str[  CAMERA_VDIG           ]="CAMERA_VDIG       "   ;
	camera_vreg_name_str[  CAMERA_VIO           ]= "CAMERA_VIO        "    ;
	camera_vreg_name_str[  CAMERA_VANA         ]=  "CAMERA_VANA       "     ;
	camera_vreg_name_str[  CAMERA_VAF           ]= "CAMERA_VAF        "    ;
	camera_vreg_name_str[  CAMERA_V_CUSTOM1      ]="CAMERA_V_CUSTOM1  "   ;
	camera_vreg_name_str[  CAMERA_V_CUSTOM2      ]="CAMERA_V_CUSTOM2  "   ;
	camera_vreg_name_str[  CAMERA_VREG_MAX       ]="CAMERA_VREG_MAX   "   ;
	
	
	if(power_setting_array->size)
	{
		DEBUG_INFO(".size = %d,\n", power_setting_array->size);
		for(i=0;i<power_setting_array->size;i++)
		{
			DEBUG_INFO(".power_setting_a = {\n");
			DEBUG_INFO("	.seq_type = %d,  // %s\n", power_setting_array->power_setting_a[i].seq_type, camera_power_seq_type_str[power_setting_array->power_setting_a[i].seq_type]);
            DEBUG_INFO("	.seq_val = %d, //%s\n",  power_setting_array->power_setting_a[i].seq_val, camera_vreg_name_str[power_setting_array->power_setting_a[i].seq_val]);
            DEBUG_INFO("	.config_val = %d,\n", power_setting_array->power_setting_a[i].config_val);
            DEBUG_INFO("	.delay = %d,\n", power_setting_array->power_setting_a[i].delay);
			DEBUG_INFO("},\n");
		}
	} else {
		DEBUG_INFO("power_setting not found!\n");
	}
	
	if(power_setting_array->size_down)
	{
		DEBUG_INFO(".size_down = %d,\n", power_setting_array->size_down);
		for(i=0;i<power_setting_array->size_down;i++)
		{
			DEBUG_INFO(".power_down_setting_a = {\n");
			DEBUG_INFO("	.seq_type = %d,  // %s\n", power_setting_array->power_down_setting_a[i].seq_type, camera_power_seq_type_str[power_setting_array->power_down_setting_a[i].seq_type]);
            DEBUG_INFO("	.seq_val = %d, //%s\n",  power_setting_array->power_down_setting_a[i].seq_val,camera_vreg_name_str[power_setting_array->power_down_setting_a[i].seq_val]);
            DEBUG_INFO("	.config_val = %d,\n", power_setting_array->power_down_setting_a[i].config_val);
            DEBUG_INFO("	.delay = %d,\n", power_setting_array->power_down_setting_a[i].delay);
			DEBUG_INFO("},\n");
		}
	} else {
		DEBUG_INFO("power_setting not found!\n");
	}
}

void print_eeprom_slave_info(eeprom_lib_func_t *info)
{
	if(info==NULL||info==0) {
		DEBUG_INFO("info not found\n");
		return;
	}
	print_power_setting_array(&info->eeprom_info.power_setting_array);
	print_memory_map_array(&info->eeprom_info.mem_map_array);
	
}
	
static	char *data_type_str[16];
static	char *addr_type_str[8] ;
	
void print_init_settings_array(struct sensor_lib_reg_settings_array *init_settings_array)
{
	int i,j;

	addr_type_str[ 0     ]=" CAMERA_I2C_BYTE_ADDR = 0,     ";
	addr_type_str[ CAMERA_I2C_BYTE_ADDR     ]=" CAMERA_I2C_BYTE_ADDR = 1,     ";
	addr_type_str[ CAMERA_I2C_WORD_ADDR        ]=" CAMERA_I2C_WORD_ADDR,         ";
	addr_type_str[ CAMERA_I2C_3B_ADDR         ]=" CAMERA_I2C_3B_ADDR,           ";
	addr_type_str[ CAMERA_I2C_ADDR_TYPE_MAX   ]=" CAMERA_I2C_ADDR_TYPE_MAX,     ";

	data_type_str[ 0              ]="   CAMERA_I2C_BYTE_DATA = 0,                ";	
	data_type_str[ CAMERA_I2C_BYTE_DATA              ]="   CAMERA_I2C_BYTE_DATA = 1,                ";
	data_type_str[ CAMERA_I2C_WORD_DATA                  ]="   CAMERA_I2C_WORD_DATA,                    ";
	data_type_str[ CAMERA_I2C_DWORD_DATA                 ]="   CAMERA_I2C_DWORD_DATA,                   ";
	data_type_str[ CAMERA_I2C_SET_BYTE_MASK              ]="   CAMERA_I2C_SET_BYTE_MASK,                ";
	data_type_str[ CAMERA_I2C_UNSET_BYTE_MASK            ]="   CAMERA_I2C_UNSET_BYTE_MASK,              ";
	data_type_str[ CAMERA_I2C_SET_WORD_MASK              ]="   CAMERA_I2C_SET_WORD_MASK,                ";
	data_type_str[ CAMERA_I2C_UNSET_WORD_MASK           ]="   CAMERA_I2C_UNSET_WORD_MASK,              ";
	data_type_str[ CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA   ]="   CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA,     ";
	data_type_str[ CAMERA_I2C_DATA_TYPE_MAX              ]="   CAMERA_I2C_DATA_TYPE_MAX,                ";
 
	if(init_settings_array->size == 0) {
		init_settings_array->size = 8;
		DEBUG_INFO("force set size = 8\n");
	}
	
	if(init_settings_array->size)
	{
		DEBUG_INFO(".size = %d,\n", init_settings_array->size);
		for(i=0;i<init_settings_array->size;i++)
		{
			DEBUG_INFO("    .reg_settings = {\n");
			DEBUG_INFO("	.addr_type = 0x%x, //%s\n", init_settings_array->reg_settings[i].addr_type, addr_type_str[init_settings_array->reg_settings[i].addr_type &0xf]);
			DEBUG_INFO("	.data_type = 0x%x, //%s\n", init_settings_array->reg_settings[i].data_type, data_type_str[init_settings_array->reg_settings[i].data_type &0xf]);			
			DEBUG_INFO("	.delay = 0x%x,\n", init_settings_array->reg_settings[i].delay);				
			DEBUG_INFO("	.size = %d,\n", init_settings_array->reg_settings[i].size);
			DEBUG_INFO("	.reg_setting_a = {  // index %d, have %d reg max\n", i,MAX_SENSOR_SETTING_I2C_REG);
			for(j=0;j<MAX_SENSOR_SETTING_I2C_REG;j++) {
				if(init_settings_array->reg_settings[i].reg_setting_a[j].reg_addr == 0) {
					DEBUG_INFO("     	{},\n");
					break;
				}
#if 0				
				DEBUG_INFO("     	{\n");
				DEBUG_INFO("	.reg_addr = 0x%x,\n", init_settings_array->reg_settings[i].reg_setting_a[j].reg_addr);
				DEBUG_INFO("	.reg_data = 0x%x,\n", init_settings_array->reg_settings[i].reg_setting_a[j].reg_data);			
				DEBUG_INFO("	.delay = 0x%x,\n", init_settings_array->reg_settings[i].reg_setting_a[j].delay);				
//				DEBUG_INFO("	.size = %d,\n", init_settings_array->reg_settings[i].reg_setting_a[j].size);			
				DEBUG_INFO("     	},\n");
#else
				DEBUG_INFO("     	{");
				DEBUG_INFO("0x%x,", init_settings_array->reg_settings[i].reg_setting_a[j].reg_addr);
				DEBUG_INFO("0x%x,", init_settings_array->reg_settings[i].reg_setting_a[j].reg_data);			
				DEBUG_INFO("0x%x,", init_settings_array->reg_settings[i].reg_setting_a[j].delay);				
//				DEBUG_INFO("	.size = %d,\n", init_settings_array->reg_settings[i].reg_setting_a[j].size);			
				DEBUG_INFO("},\n");				
#endif

			}
			DEBUG_INFO("    }, //total %d entries\n",j);
		}
	} else {
		DEBUG_INFO("reg_settings not found!\n");
	}

};

void print_reg_array(struct camera_i2c_reg_setting_array reg_settings[], char* name)
{
	int i=0,j;

	
			DEBUG_INFO("%s = {\n",name);
			DEBUG_INFO("	.reg_setting_a = {  // index %d\n", i);
		//	DEBUG_INFO("		.size = %d,  // index %d\n", i);
			for(j=0;j<MAX_SENSOR_SETTING_I2C_REG;j++) {
				if(reg_settings[i].reg_setting_a[j].reg_addr == 0) {
					DEBUG_INFO("   		  	{},\n");
					break;
				}
#if 0				
				DEBUG_INFO("    	 	{\n");
				DEBUG_INFO("		.reg_addr = 0x%x,\n", reg_settings[i].reg_setting_a[j].reg_addr);
				DEBUG_INFO("		.reg_data = 0x%x,\n", reg_settings[i].reg_setting_a[j].reg_data);			
				DEBUG_INFO("		.delay = 0x%x,\n", reg_settings[i].reg_setting_a[j].delay);				
//				DEBUG_INFO("	.size = %d,\n", init_settings_array->reg_settings[i].reg_setting_a[j].size);			
				DEBUG_INFO("  	   	},\n");
#else
				DEBUG_INFO("    	 	{");
				DEBUG_INFO("0x%x,", reg_settings[i].reg_setting_a[j].reg_addr);
				DEBUG_INFO("0x%x,", reg_settings[i].reg_setting_a[j].reg_data);			
				DEBUG_INFO("0x%x,", reg_settings[i].reg_setting_a[j].delay);				
//				DEBUG_INFO("	.size = %d,\n", init_settings_array->reg_settings[i].reg_setting_a[j].size);			
				DEBUG_INFO("},\n");
#endif
			}
			DEBUG_INFO("	},\n");
			DEBUG_INFO("}, //end of %s\n",name);
}

void print_settings_array(struct sensor_lib_reg_settings_array *init_settings_array, char* name)
{
	if(init_settings_array ==NULL )
		return;
		
	if(init_settings_array->size == 0) {
		hexDump(name, init_settings_array, 128);
	}
	else if (1){	
		DEBUG_INFO(".%s = {\n", name);
		print_init_settings_array(init_settings_array);
		DEBUG_INFO("},\n");
	}
}

static char* fill_exp_enum[8] = {
	"FILL_CUSTOM_IN_LIB         ",
	"FILL_2B_GAIN_2B_IT_2B_FLL  ",
	"FILL_2B_GAIN_3B_IT_2B_FLL  ",
	"FILL_CUSTOM_IN_CMN_LIB     ",
	
};

static char* calc_exp_enum[8] = {
	"CALC_CUSTOM_IN_LIB",
	"CALC_SMIA_TYPE_METHOD",
	"CALC_CUSTOM_IN_CMN_LIB",
	
};
	
static char *sensor_stats_types_enum[8];	
static char *sensing_method_enum[8];
	
void print_sensor_slave_info(sensor_lib_t *info)
{
	int i = 0;
	
	fill_exp_enum[	FILL_CUSTOM_IN_LIB             ] = "FILL_CUSTOM_IN_LIB          ";
	fill_exp_enum[	FILL_2B_GAIN_2B_IT_2B_FLL      ] = "FILL_2B_GAIN_2B_IT_2B_FLL   ";
	fill_exp_enum[	FILL_2B_GAIN_3B_IT_2B_FLL      ] = "FILL_2B_GAIN_3B_IT_2B_FLL   ";
	fill_exp_enum[	FILL_CUSTOM_IN_CMN_LIB         ] = "FILL_CUSTOM_IN_CMN_LIB      ";

	calc_exp_enum[CALC_CUSTOM_IN_LIB]     			= "CALC_CUSTOM_IN_LIB";
	calc_exp_enum[CALC_SMIA_TYPE_METHOD]  			= "CALC_SMIA_TYPE_METHOD";
	calc_exp_enum[CALC_CUSTOM_IN_CMN_LIB] 			= "CALC_CUSTOM_IN_CMN_LIB";
	
	sensor_stats_types_enum[0                   ]=" 0                     ";
	sensor_stats_types_enum[HDR_STATS           ]=" HDR_STATS             ";
	sensor_stats_types_enum[PD_STATS            ]=" PD_STATS              ";
	sensor_stats_types_enum[STAGGERED_HDR_FRAME	]=" STAGGERED_HDR_FRAME	  ";
	

	sensing_method_enum[  SENSOR_SMETHOD_NOT_DEFINED                     ] = " SENSOR_SMETHOD_NOT_DEFINED                       ";
	sensing_method_enum[  SENSOR_SMETHOD_ONE_CHIP_COLOR_AREA_SENSOR      ] = " SENSOR_SMETHOD_ONE_CHIP_COLOR_AREA_SENSOR        ";
	sensing_method_enum[  SENSOR_SMETHOD_TWO_CHIP_COLOR_AREA_SENSOR      ] = " SENSOR_SMETHOD_TWO_CHIP_COLOR_AREA_SENSOR        ";
	sensing_method_enum[  SENSOR_SMETHOD_THREE_CHIP_COLOR_AREA_SENSOR    ] = " SENSOR_SMETHOD_THREE_CHIP_COLOR_AREA_SENSOR      ";
	sensing_method_enum[  SENSOR_SMETHOD_COLOR_SEQ_AREA_SENSOR           ] = " SENSOR_SMETHOD_COLOR_SEQ_AREA_SENSOR             ";
	sensing_method_enum[  SENSOR_SMETHOD_TRILINEAR_SENSOR                ] = " SENSOR_SMETHOD_TRILINEAR_SENSOR                  ";
	sensing_method_enum[  SENSOR_SMETHOD_COLOR_SEQ_LINEAR_SENSOR         ] = " SENSOR_SMETHOD_COLOR_SEQ_LINEAR_SENSOR           ";

										
	if(info==NULL||info==0) {
		DEBUG_INFO("info not found\n");
		return;
	} 
	
	#define DEBUG_INFO_1(a) do {DEBUG_INFO("+0x%x \t %s:\t\t", ((void*)&(info->a) - (void*)&(info->sensor_slave_info)), #a); hexDump(NULL, (void*)(&info->a),16);} while (0); 
	DEBUG_INFO_1(sensor_slave_info                        );
	DEBUG_INFO_1(sensor_slave_info.slave_addr             );
	DEBUG_INFO_1(sensor_slave_info.data_type              );
	DEBUG_INFO_1(sensor_slave_info.sensor_id_info         );
	DEBUG_INFO_1(sensor_slave_info.power_setting_array    );
	DEBUG_INFO_1(sensor_slave_info.is_init_params_valid   );
	DEBUG_INFO_1(sensor_output                            );
	DEBUG_INFO_1(output_reg_addr                          );
	DEBUG_INFO_1(exp_gain_info                            );
	DEBUG_INFO_1(aec_info                                 );
	DEBUG_INFO_1(sensor_num_frame_skip                    );
	DEBUG_INFO_1(sensor_num_HDR_frame_skip                );
	DEBUG_INFO_1(sensor_max_pipeline_frame_delay          );
	DEBUG_INFO_1(sensor_property                          );
	DEBUG_INFO_1(pixel_array_size_info                    );
	DEBUG_INFO_1(color_level_info                         );
	DEBUG_INFO_1(sensor_stream_info_array                 );
	DEBUG_INFO_1(start_settings                           );
	DEBUG_INFO_1(stop_settings                            );
	DEBUG_INFO_1(groupon_settings                         );
	DEBUG_INFO_1(groupoff_settings                        );
	DEBUG_INFO_1(embedded_data_enable_settings            );
	DEBUG_INFO_1(embedded_data_disable_settings           );
	DEBUG_INFO_1(aec_enable_settings                      );
	DEBUG_INFO_1(aec_disable_settings                     );
	DEBUG_INFO_1(dualcam_master_settings                  );
	DEBUG_INFO_1(test_pattern_info                        );
	DEBUG_INFO_1(effect_info                              );
	DEBUG_INFO_1(init_settings_array                      );
	DEBUG_INFO_1(res_settings_array                       );
	DEBUG_INFO_1(out_info_array                           );
	

	if (info->sensor_slave_info.power_setting_array.size == 0) {
		struct camera_sensor_slave_info_mod* slave_info = (struct camera_sensor_slave_info_mod*)&info->sensor_slave_info;
		DEBUG_INFO("//check with modified power_setting location @0x%x. and shifted 0x%x\n", (void*)&slave_info->power_setting_array - (void*)&info->sensor_slave_info, (void*)&slave_info->power_setting_array - (void*)&info->sensor_slave_info.power_setting_array);
		hexDump("power_setting", &slave_info->power_setting_array, sizeof(struct camera_power_setting_array));
		print_power_setting_array(&slave_info->power_setting_array);
	} else {
		DEBUG_INFO("//check with original power_setting location.\n");
		hexDump("power_setting", &info->sensor_slave_info.power_setting_array, sizeof(struct camera_power_setting_array));
		print_power_setting_array(&info->sensor_slave_info.power_setting_array);
	}
			
	hexDump("sensor_output", &info->sensor_output, 16);
	hexDump("output_reg_addr", &info->output_reg_addr, 16);
	DEBUG_INFO(".output_reg_addr.x_output           = 0x%x;\n", info->output_reg_addr.x_output);
	DEBUG_INFO(".output_reg_addr.y_output           = 0x%x;\n", info->output_reg_addr.y_output);
	DEBUG_INFO(".output_reg_addr.line_length_pclk   = 0x%x;\n", info->output_reg_addr.line_length_pclk);
	DEBUG_INFO(".output_reg_addr.frame_length_lines = 0x%x;\n", info->output_reg_addr.frame_length_lines);
				
	DEBUG_INFO(".exp_gain_info.coarse_int_time_addr              = 0x%x ;\n"      ,  info->exp_gain_info.coarse_int_time_addr            )    ;
	DEBUG_INFO(".exp_gain_info.short_coarse_int_time_addr        = 0x%x ;\n"      ,  info->exp_gain_info.short_coarse_int_time_addr      )    ;
	DEBUG_INFO(".exp_gain_info.global_gain_addr                  = 0x%x ;\n"      ,  info->exp_gain_info.global_gain_addr                )    ;
	DEBUG_INFO(".exp_gain_info.short_global_gain_addr            = 0x%x ;\n"      ,  info->exp_gain_info.short_global_gain_addr          )    ;
	DEBUG_INFO(".exp_gain_info.dig_gain_r_addr                   = 0x%x ;\n"      ,  info->exp_gain_info.dig_gain_r_addr                 )    ;
	DEBUG_INFO(".exp_gain_info.dig_gain_gr_addr                  = 0x%x ;\n"      ,  info->exp_gain_info.dig_gain_gr_addr                )    ;
	DEBUG_INFO(".exp_gain_info.dig_gain_b_addr                   = 0x%x ;\n"      ,  info->exp_gain_info.dig_gain_b_addr                 )    ;
	DEBUG_INFO(".exp_gain_info.dig_gain_gb_addr                  = 0x%x ;\n"      ,  info->exp_gain_info.dig_gain_gb_addr                )    ;
	DEBUG_INFO(".exp_gain_info.vert_offset                       = 0x%x ;\n"      ,  info->exp_gain_info.vert_offset                     )    ;

	DEBUG_INFO(".aec_info.min_gain                         = %f ;\n",  info->aec_info.min_gain                              );
	DEBUG_INFO(".aec_info.max_gain                         = %f ;\n",  info->aec_info.max_gain                              );
	DEBUG_INFO(".aec_info.max_analog_gain                  = %f ;\n",  info->aec_info.max_analog_gain                       );
	DEBUG_INFO(".aec_info.min_analog_gain                  = %f ;\n",  info->aec_info.min_analog_gain                       );
	DEBUG_INFO(".aec_info.min_digital_gain                 = %f ;\n",  info->aec_info.min_digital_gain                      );
	DEBUG_INFO(".aec_info.max_digital_gain                 = %f ;\n",  info->aec_info.max_digital_gain                      );
	DEBUG_INFO(".aec_info.dig_gain_decimator               = %d ;\n",  info->aec_info.dig_gain_decimator                    );
	DEBUG_INFO(".aec_info.max_linecount                    = %d ;\n",  info->aec_info.max_linecount                         );
	
	DEBUG_INFO(".aec_info.smia_type_gain_coeff.m0            = %d ;\n",  info->aec_info.smia_type_gain_coeff.m0                 );
	DEBUG_INFO(".aec_info.smia_type_gain_coeff.m1            = %d ;\n",  info->aec_info.smia_type_gain_coeff.m1                 );
	DEBUG_INFO(".aec_info.smia_type_gain_coeff.m2            = %d ;\n",  info->aec_info.smia_type_gain_coeff.m2                 );
	DEBUG_INFO(".aec_info.smia_type_gain_coeff.c0            = %d ;\n",  info->aec_info.smia_type_gain_coeff.c0                 );
	DEBUG_INFO(".aec_info.smia_type_gain_coeff.c1            = %d ;\n",  info->aec_info.smia_type_gain_coeff.c1                 );
	DEBUG_INFO(".aec_info.smia_type_gain_coeff.c2            = %d ;\n",  info->aec_info.smia_type_gain_coeff.c2                 );
	//DEBUG_INFO(".aec_info.smia_type_gain_coeff.            = %d ;\n",  info->aec_info.smia_type_gain_coeff                  );

	DEBUG_INFO(".aec_info.svhdr_use_separate_gain          = %d ;\n",  info->aec_info.svhdr_use_separate_gain               );
	DEBUG_INFO(".aec_info.svhdr_use_separate_limits        = %d ;\n",  info->aec_info.svhdr_use_separate_limits             );
	DEBUG_INFO(".aec_info.min_line_cnt[0]                  = %d ;\n",  info->aec_info.min_line_cnt[0]                       );
	DEBUG_INFO(".aec_info.min_line_cnt[1]                  = %d ;\n",  info->aec_info.min_line_cnt[1]                       );
	DEBUG_INFO(".aec_info.min_line_cnt[2]                  = %d ;\n",  info->aec_info.min_line_cnt[2]                       );
	DEBUG_INFO(".aec_info.min_line_cnt[3]                  = %d ;\n",  info->aec_info.min_line_cnt[3]                       );
	DEBUG_INFO(".aec_info.max_line_cnt[0]                  = %d ;\n",  info->aec_info.max_line_cnt[0]                       );
	DEBUG_INFO(".aec_info.max_line_cnt[1]                  = %d ;\n",  info->aec_info.max_line_cnt[1]                       );
	DEBUG_INFO(".aec_info.max_line_cnt[2]                  = %d ;\n",  info->aec_info.max_line_cnt[2]                       );
	DEBUG_INFO(".aec_info.max_line_cnt[3]                  = %d ;\n",  info->aec_info.max_line_cnt[3]                       );



	DEBUG_INFO(".sensor_num_frame_skip           = 0x%x ;\n", info->sensor_num_frame_skip);
	DEBUG_INFO(".ensor_num_HDR_frame_skip        = 0x%x ;\n", info->sensor_num_HDR_frame_skip);
	DEBUG_INFO(".sensor_max_pipeline_frame_delay = 0x%x ;\n", info->sensor_max_pipeline_frame_delay);
	
	DEBUG_INFO(".sensor_property.crop_factor    =%f ;\n" ,                                info->sensor_property.crop_factor     );
	DEBUG_INFO(".sensor_property.pix_size       =%f ;\n" ,                                info->sensor_property.pix_size        );
	DEBUG_INFO(".sensor_property.sensing_method =%s ;\n" , sensing_method_enum[           info->sensor_property.sensing_method &0xf] );

	DEBUG_INFO(".pixel_array_size_info.active_array_size.width  = 0x%x;\n", info->pixel_array_size_info.active_array_size.width);
	DEBUG_INFO(".pixel_array_size_info.active_array_size.height = 0x%x;\n", info->pixel_array_size_info.active_array_size.height);
	
	DEBUG_INFO(".color_level_info.white_level      = %d ; \n",         info->color_level_info.white_level   );
	DEBUG_INFO(".color_level_info.r_pedestal       = %d ; \n",         info->color_level_info.r_pedestal    );
	DEBUG_INFO(".color_level_info.gr_pedestal      = %d ; \n",         info->color_level_info.gr_pedestal   );
	DEBUG_INFO(".color_level_info.gb_pedestal      = %d ; \n",         info->color_level_info.gb_pedestal   );
	DEBUG_INFO(".color_level_info.b_pedestal       = %d ; \n",         info->color_level_info.b_pedestal    );

	print_reg_array(&info->start_settings,                 ".start_settings");
	print_reg_array(&info->stop_settings,                  ".stop_settings");
	print_reg_array(&info->groupon_settings,               ".groupon_settings");
	print_reg_array(&info->groupoff_settings,              ".groupoff_settings");
	print_reg_array(&info->embedded_data_enable_settings,  ".embedded_data_enable_settings");
	print_reg_array(&info->embedded_data_disable_settings, ".embedded_data_disable_settings");
	print_reg_array(&info->aec_enable_settings,            ".aec_enable_settings");
	print_reg_array(&info->aec_disable_settings,           ".aec_disable_settings");
	print_reg_array(&info->dualcam_master_settings,        ".dualcam_master_settings");
	print_reg_array(&info->dualcam_slave_settings,         ".dualcam_slave_settings");

	DEBUG_INFO("//init_settings_array offset = 0x%x;\n", (int)((void*)&info->init_settings_array - (void*)&info->sensor_slave_info));
	print_settings_array(&info->init_settings_array, "init_settings_array");
	DEBUG_INFO("//res_settings_array offset = 0x%x;\n", (int)((void*)&info->res_settings_array - (void*)&info->sensor_slave_info));  
	print_settings_array(&info->res_settings_array, "res_settings_array");

	for (i=0;(info->out_info_array.size == 0) && (i<MAX_RESOLUTION_MODES) || (i < info->out_info_array.size);i++){
		DEBUG_INFO(".out_info_array.out_info[%d].x_output            = %d    ;  \n", i, info->out_info_array.out_info[i].x_output             );
		DEBUG_INFO(".out_info_array.out_info[%d].y_output            = %d    ;  \n", i, info->out_info_array.out_info[i].y_output             );
		DEBUG_INFO(".out_info_array.out_info[%d].line_length_pclk    = %d    ;  \n", i, info->out_info_array.out_info[i].line_length_pclk     );
		DEBUG_INFO(".out_info_array.out_info[%d].frame_length_lines  = %d    ;  \n", i, info->out_info_array.out_info[i].frame_length_lines   );
		DEBUG_INFO(".out_info_array.out_info[%d].vt_pixel_clk        = %d    ;  \n", i, info->out_info_array.out_info[i].vt_pixel_clk         );
		DEBUG_INFO(".out_info_array.out_info[%d].op_pixel_clk        = %d    ;  \n", i, info->out_info_array.out_info[i].op_pixel_clk         );
		DEBUG_INFO(".out_info_array.out_info[%d].binning_factor      = %d    ;  \n", i, info->out_info_array.out_info[i].binning_factor       );
		DEBUG_INFO(".out_info_array.out_info[%d].binning_method      = %d    ;  \n", i, info->out_info_array.out_info[i].binning_method       );
		DEBUG_INFO(".out_info_array.out_info[%d].min_fps             = %d    ;  \n", i, info->out_info_array.out_info[i].min_fps              );
		DEBUG_INFO(".out_info_array.out_info[%d].max_fps             = %d    ;  \n", i, info->out_info_array.out_info[i].max_fps              );
		DEBUG_INFO(".out_info_array.out_info[%d].mode                = %d    ;  \n", i, info->out_info_array.out_info[i].mode                 );
		DEBUG_INFO(".out_info_array.out_info[%d].offset_x            = %d    ;  \n", i, info->out_info_array.out_info[i].offset_x             );
		DEBUG_INFO(".out_info_array.out_info[%d].offset_y            = %d    ;  \n", i, info->out_info_array.out_info[i].offset_y             );
		DEBUG_INFO(".out_info_array.out_info[%d].scale_factor        = %d    ;  \n", i, info->out_info_array.out_info[i].scale_factor         );
		DEBUG_INFO(".out_info_array.out_info[%d].is_pdaf_supported   = %d    ;  \n", i, info->out_info_array.out_info[i].is_pdaf_supported    );
		DEBUG_INFO(".out_info_array.out_info[%d].min_hbi             = %d    ;  \n", i, info->out_info_array.out_info[i].min_hbi              );
		DEBUG_INFO(".out_info_array.out_info[%d].min_vbi             = %d    ;  \n", i, info->out_info_array.out_info[i].min_vbi              );
		DEBUG_INFO(".out_info_array.out_info[%d].custom_format.enable   		   = %d    ;  \n", i, info->out_info_array.out_info[i].custom_format.enable   		 ); 
		DEBUG_INFO(".out_info_array.out_info[%d].custom_format.subframes_cnt    = %d    ;  \n", i, info->out_info_array.out_info[i].custom_format.subframes_cnt   );
		DEBUG_INFO(".out_info_array.out_info[%d].custom_format.start_x          = %d    ;  \n", i, info->out_info_array.out_info[i].custom_format.start_x         );
		DEBUG_INFO(".out_info_array.out_info[%d].custom_format.start_y          = %d    ;  \n", i, info->out_info_array.out_info[i].custom_format.start_y         );
		DEBUG_INFO(".out_info_array.out_info[%d].custom_format.width            = %d    ;  \n", i, info->out_info_array.out_info[i].custom_format.width           );
		DEBUG_INFO(".out_info_array.out_info[%d].custom_format.height           = %d    ;  \n", i, info->out_info_array.out_info[i].custom_format.height          );
	}


	DEBUG_INFO(".csi_params.lane_cnt      = %d;\n" ,     info->csi_params.lane_cnt      );
	DEBUG_INFO(".csi_params.settle_cnt    = %d;\n" ,     info->csi_params.settle_cnt    );
	DEBUG_INFO(".csi_params.is_csi_3phase = %d;\n" ,     info->csi_params.is_csi_3phase );
	DEBUG_INFO(".csid_lut_params_array.size = %d;\n" ,   info->csid_lut_params_array.size  );  
	DEBUG_INFO(".crop_params_array.size     = %d;\n" ,   info->crop_params_array.size      );   

	DEBUG_INFO(".exposure_func_table.calc_exp_array_type = %d;//%s ;\n", info->exposure_func_table.calc_exp_array_type, calc_exp_enum[info->exposure_func_table.calc_exp_array_type &0xf]);
 	DEBUG_INFO(".exposure_func_table.fill_exp_array_type = %d;//%s ;\n", info->exposure_func_table.fill_exp_array_type, fill_exp_enum[info->exposure_func_table.fill_exp_array_type &0xf]);
 
	DEBUG_INFO(".meta_data_out_info_array.size = %d ;\n", info->meta_data_out_info_array.size);
 
 	for (i=0;(info->meta_data_out_info_array.size == 0) && (i<MAX_RESOLUTION_MODES) || (i < info->meta_data_out_info_array.size);i++){
		DEBUG_INFO(".meta_data_out_info_array.meta_data_out_info[%d].width           = %d;\n" , i, info->meta_data_out_info_array.meta_data_out_info[i].width         );
		DEBUG_INFO(".meta_data_out_info_array.meta_data_out_info[%d].height          = %d;\n" , i, info->meta_data_out_info_array.meta_data_out_info[i].height        );
		DEBUG_INFO(".meta_data_out_info_array.meta_data_out_info[%d].stats_type      = %d;//%s\n" , i, info->meta_data_out_info_array.meta_data_out_info[i].stats_type, sensor_stats_types_enum[info->meta_data_out_info_array.meta_data_out_info[i].stats_type & 0xf]    );
		DEBUG_INFO(".meta_data_out_info_array.meta_data_out_info[%d].dt              = %d;\n" , i, info->meta_data_out_info_array.meta_data_out_info[i].dt            );
		DEBUG_INFO(".meta_data_out_info_array.meta_data_out_info[%d].cid             = %d;\n" , i, info->meta_data_out_info_array.meta_data_out_info[i].cid           );
	}
	
	for (i=0;(info->optical_black_region_info.size == 0) && (i<MAX_SENSOR_OPTICAL_BLACK_REGION) || (i < info->optical_black_region_info.size);i++){

		DEBUG_INFO(".optical_black_region_info.optical_black_region[%d].x_start      = 0x%x; \n"  ,i, info->optical_black_region_info.optical_black_region[i].x_start    );
		DEBUG_INFO(".optical_black_region_info.optical_black_region[%d].y_start      = 0x%x; \n"  ,i, info->optical_black_region_info.optical_black_region[i].y_start    );
		DEBUG_INFO(".optical_black_region_info.optical_black_region[%d].width        = 0x%x; \n"  ,i, info->optical_black_region_info.optical_black_region[i].width      );
		DEBUG_INFO(".optical_black_region_info.optical_black_region[%d].height       = 0x%x; \n"  ,i, info->optical_black_region_info.optical_black_region[i].height     );
	}	

	DEBUG_INFO(".sensor_capability = 0x%lx;\n", info->sensor_capability);
  	DEBUG_INFO(".adc_readout_time     = %ld;\n", info->adc_readout_time);
	DEBUG_INFO(".sensor_num_fast_aec_frame_skip =%d;\n", info->sensor_num_fast_aec_frame_skip);
	
	hexDump(".app_delay", info->app_delay, SENSOR_DELAY_MAX);
	
	DEBUG_INFO(".noise_coeff.gradient_S        = %f;\n", info->noise_coeff.gradient_S     );
	DEBUG_INFO(".noise_coeff.offset_S          = %f;\n", info->noise_coeff.offset_S       );
	DEBUG_INFO(".noise_coeff.gradient_O        = %f;\n", info->noise_coeff.gradient_O     );
	DEBUG_INFO(".noise_coeff.offset_O          = %f;\n", info->noise_coeff.offset_O       );
	
	DEBUG_INFO(".test_pattern_info.solid_mode_addr.r_addr  = 0x%x ;\n", info->test_pattern_info.solid_mode_addr.r_addr);
	DEBUG_INFO(".test_pattern_info.solid_mode_addr.gr_addr = 0x%x ;\n", info->test_pattern_info.solid_mode_addr.gr_addr);
	DEBUG_INFO(".test_pattern_info.solid_mode_addr.gb_addr = 0x%x ;\n", info->test_pattern_info.solid_mode_addr.gb_addr);
	DEBUG_INFO(".test_pattern_info.solid_mode_addr.b_addr  = 0x%x ;\n", info->test_pattern_info.solid_mode_addr.b_addr);	

	for (i=0;(info->test_pattern_info.size == 0) && (i<SENSOR_TEST_PATTERN_MAX) || (i < info->test_pattern_info.size);i++){
		DEBUG_INFO(".test_pattern_info.test_pattern_settings[%d].mode = %d ;\n", i, info->test_pattern_info.test_pattern_settings[i].mode       );
		//DEBUG_INFO(".test_pattern_info.test_pattern_settings[%d].solid_mode_addr = 0x%x ;\n", test_pattern_info.test_pattern_settings[%d].solid_mode_addr      );
		DEBUG_INFO(".test_pattern_info.test_pattern_settings[%d]", i);
		print_reg_array(&info->test_pattern_info.test_pattern_settings[i].settings,                 ".settings");
	}

	DEBUG_INFO("//DEBUG\n");

	DEBUG_INFO("//info size = %d\n", sizeof(sensor_lib_t));


#if 0
	if(1) {
		int reg_offset_value[30] = {
			0x4008    -0x4008,
			0x9e28    -0x4008,
			0x9fac    -0x4008,
			0x9fbc    -0x4008,
			0x9fc4    -0x4008,
			0x9f20    -0x4008,
			0xa044    -0x4008,
			0xa060    -0x4008,
			0xa26c    -0x4008,
			0xa59c    -0x4008,
			0xa8cc    -0x4008,
			0xabfc    -0x4008,
			0xaf2c    -0x4008,
			0xb25c    -0x4008,
			0xb58c    -0x4008,
			0xb8bc    -0x4008,
			0xbbec    -0x4008,
			0xc24c    -0x4008,
			0xdf30    -0x4008,
			0x60090   -0x4008,
			0x60094   -0x4008,
			0x65e54   -0x4008,
			0x65e58   -0x4008,
			0x6bc28   -0x4008,
			0x719f8   -0x4008,
			0x777c8   -0x4008,
			0x7d598   -0x4008,
			0x83368   -0x4008,
			0x89138   -0x4008,
		};	
		char* reg_offset_str[30] = {
			"slave_info                        ",
			"??pattern                         ",
			"sensor_output                     ",
			"output_reg_addr                   ",
			"exp_gain_info ??                  ",
			"sensor_num_frame_skip             ",
			"pixel_array_size_info             ",
			"sensor_stream_info_array          ",
			"start_settings                    ",
			"stop  +0x330                      ",
			"groupon_settings                  ",
			"groupoff                          ",
			"embedded_data_enable_settings;    ",
			"embedded_data_disable_settings    ",
			"aec_enable_settings;              ",
			"aec_disable_settings;             ",
			"dualcam_settings;                 ",
			"test_pattern_info??               ",
			"init_settings_array               ",
			" init_settings_array.size         ",
			" res_settings_array               ",
			" res_settings_array.size          ",
			" res_settings_array.addr_type     ",
			" ...addr_type                     ",
			" ...addr_type                     ",
			" ...addr_type                     ",
			" ...addr_type                     ",
			" ...addr_type                     ",
			" ...addr_type                     ",
			"",
		};
		
		DEBUG_INFO("patched offset value as:\n");
		i=0;
		while(reg_offset_str[i]!="") {
			DEBUG_INFO("0x%x\t \t %s", reg_offset_value[i], reg_offset_str[i]);
			hexDump(NULL, (void*)(&info->sensor_slave_info) + reg_offset_value[i],16);
			i++;
		}
	
		DEBUG_INFO("revised *sensor_slave_info[0] = 0, apply offset");
		print_settings_array((struct sensor_lib_reg_settings_array *)((void*)info + 0x9f28), "init_settings_array");
		print_settings_array((struct sensor_lib_reg_settings_array *)((void*)info + 0x5c08c), "res_settings_array");
	}

#endif
	//hexDump("dummy", info->dummy, sizeof(info->dummy));
	hexDump("power_setting_array", &info->sensor_slave_info.power_setting_array, 32);

	DEBUG_INFO("revised offset end\n");
}

void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
    printf("Caught segfault at address %p\n", si->si_addr);
	sensor_unload_library(&sensor_lib_params_0);
    exit(0);
}

int main(int argc, char *argv[]){
	int i;
	static char *sensor = "imx350\0";
    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags   = SA_SIGINFO;
	
	sigaction(SIGSEGV, &sa, NULL);

	ALOG("main\n");
	
	DEBUG_INFO("Hello world %s %s\n", argv[0], argv[1]);

	if (argc > 1) {
		sensor = argv[1];
		DEBUG_INFO("usage: %s [cmos|imx350]\n", argv[0]);
		DEBUG_INFO("default 'imx350' for libmmcamera_imx350.so\n");
		DEBUG_INFO("e.g. %s s5k3p3sm for libmmcamera_s5k3p3sm.so\n", argv[0]);
	}

	i = sensor_load_library(sensor, &sensor_lib_params_0);
	DEBUG_INFO("sensor_load_library, return %d\n", i);
	if(i==0) {
		if (sensor_lib_params_0.sensor_lib_ptr)
		{
			hexDump("msm_camera_sensor_slave_info", sensor_lib_params_0.sensor_lib_ptr, 256);
			print_sensor_slave_info(sensor_lib_params_0.sensor_lib_ptr);
		}

	}
	else
		DEBUG_INFO("no info\n");
		
	DEBUG_INFO("b4 load eeprom lib\n");	
	i = eeprom_load_library(sensor, &sensor_lib_params_0);
	DEBUG_INFO("eeprom_load_library, return %d\n", i);
	
	if(i==0) {	
		if (sensor_lib_params_0.eeprom_lib_ptr)
		{
			hexDump("msm_camera_info", sensor_lib_params_0.eeprom_lib_ptr, 256);
			hexDump("msm_camera_eeprom_info", &sensor_lib_params_0.eeprom_lib_ptr->eeprom_info, 256);
			print_eeprom_slave_info(sensor_lib_params_0.eeprom_lib_ptr);
		}
	}
	
exit:	
	sensor_unload_library(&sensor_lib_params_0);

	DEBUG_INFO("sensor_unload_library\n");


	return 0;
}

