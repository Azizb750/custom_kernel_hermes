// WIP by LazyC0DEr
// Don't build this
static u8 config_info[SIZE_BATINFO] = {
  0x17, 0xF3, 0x63, 0x6A, 0x6A, 0x68, 0x68, 0x65, 0x63, 0x60, 
  0x5B, 0x59, 0x65, 0x5B, 0x46, 0x41, 0x36, 0x31, 0x28, 0x27, 
  0x31, 0x35, 0x43, 0x51, 0x1C, 0x3B, 0x0B, 0x85, 0x22, 0x42, 
  0x5B, 0x82, 0x99, 0x92, 0x98, 0x96, 0x3D, 0x1A, 0x66, 0x45, 
  0x0B, 0x29, 0x52, 0x87, 0x8F, 0x91, 0x94, 0x52, 0x82, 0x8C, 
  0x92, 0x96, 0x54, 0xC2, 0xBA, 0xCB, 0x2F, 0x7D, 0x72, 0xA5, 
  0xB5, 0xC1, 0xA5, 0x49
}
static u8 config_info_des[SIZE_BATINFO] = {
  0x17, 0xF9, 0x6D, 0x6D, 0x6B, 0x67, 0x65, 0x64, 0x58, 0x6D, 
  0x6D, 0x48, 0x57, 0x5D, 0x4A, 0x43, 0x37, 0x31, 0x2B, 0x20, 
  0x24, 0x35, 0x44, 0x55, 0x20, 0x37, 0x0B, 0x85, 0x2A, 0x4A, 
  0x56, 0x68, 0x74, 0x6B, 0x6D, 0x6E, 0x3C, 0x1A, 0x5C, 0x45, 
  0x0B, 0x30, 0x52, 0x87, 0x8F, 0x91, 0x94, 0x52, 0x82, 0x8C, 
  0x92, 0x96, 0x64, 0xB4, 0xDB, 0xCB, 0x2F, 0x7D, 0x72, 0xA5, 
  0xB5, 0xC1, 0xA5, 0x42
}


static int liuchao_test_hmi_battery_version = 1;

/* */
static void hmi_get_battery_version()
{
    int i = 1;
    //	i = strtol(strstr(cmdline, "batversion=")+12, 0, 10);
    liuchao_test_hmi_battery_version = i; //COS = 1, DES = 2
}

static int cw_init(struct cw_battery *cw_bat)
{
    int ret;
    int i;
    u8 reg_val = MODE_SLEEP;
    static struct devinfo_struct *devinfo_bat = NULL;
    
    hmi_get_battery_version();
    
    devinfo_bat = kzalloc(sizeof(struct devinfo_struct), GFP_KERNEL);    
    devinfo_bat->device_type = "BATTERY";
    devinfo_bat->device_vendor = DEVINFO_NULL;
    devinfo_bat->device_ic = DEVINFO_NULL;
    devinfo_bat->device_version = DEVINFO_NULL;
    devinfo_bat->device_info = DEVINFO_NULL;
    devinfo_bat->device_used = DEVINFO_USED;
    
    
    switch (liuchao_test_hmi_battery_version){
        case: 1{
            devinfo_bat->device_module = "COS";
        }
        case :2 {
            devinfo_bat->device_module = "DES";
        }
        default:{
            devinfo_bat->device_module = "ERROR";
        }  
    }
    
    
    if (!devinfo_check_add_device(devinfo_bat)){
        kfree(devinfo_bat);
        dev_info(&cw_bat->client->dev, "free devinfo for not register into devinfo list .\n");
    }else{
        dev_info(&cw_bat->client->dev, "register devinfo into devinfo list .\n");
    }
    if ((reg_val & MODE_SLEEP_MASK) == MODE_SLEEP) {
        reg_val = MODE_NORMAL;
        ret = cw_write(cw_bat->client, REG_MODE, &reg_val);
        if (ret < 0)
            return ret;
    }
    ret = cw_read(cw_bat->client, REG_CONFIG, &reg_val);
    if (ret < 0)
        return ret;
    
    ret = cw_read(cw_bat->client, REG_CONFIG, &reg_val);
    if (ret < 0)
        return ret;
    #ifdef FG_CW2015_DEBUG
    FG_CW2015_LOG("the new ATHD have not set reg_val = 0x%x\n",reg_val);
    #endif
    if ((reg_val & 0xf8) != ATHD) {
        #ifdef FG_CW2015_DEBUG
        FG_CW2015_LOG("the new ATHD have not set\n");
        #endif
        reg_val &= 0x07;    /* clear ATHD */
        reg_val |= ATHD;    /* set ATHD */
        ret = cw_write(cw_bat->client, REG_CONFIG, &reg_val);
        FG_CW2015_LOG("cw_init 1111\n");
        if (ret < 0)
            return ret;
    }
    
    ret = cw_read(cw_bat->client, REG_CONFIG, &reg_val);
    if (ret < 0) 
        return ret;
    FG_CW2015_LOG("cw_init REG_CONFIG = %d\n",reg_val);
    
    if (!(reg_val & CONFIG_UPDATE_FLG)) {
        #ifdef FG_CW2015_DEBUG
        FG_CW2015_LOG("update flag for new battery info have not set\n");
        #endif
        ret = cw_update_config_info(cw_bat);
        if (ret < 0)
            return ret;
    } else {
        for(i = 0; i < SIZE_BATINFO; i++) { 
            ret = cw_read(cw_bat->client, (REG_BATINFO + i), &reg_val);
            if (ret < 0)
                return ret;
            
            if (2 == liuchao_test_hmi_battery_version){
                if (config_info_des[i] != reg_val)
                    break;
            
            }else{
                if (config_info[i] != reg_val)
                    break;
            }
        }
        
        if (i != SIZE_BATINFO) {
            #ifdef FG_CW2015_DEBUG
            FG_CW2015_LOG("update flag for new battery info have not set\n"); 
            #endif
            ret = cw_update_config_info(cw_bat);
            if (ret < 0)
                return ret;
        }
    }
    
    for (i = 0; i < 30; i++) {
        ret = cw_read(cw_bat->client, REG_SOC, &reg_val);
        if (ret < 0)
            return ret;
        else if (reg_val <= 0x64) 
            break;
        
        msleep(100);
        if (i > 25){
            #ifdef FG_CW2015_DEBUG
            FG_CW2015_ERR("cw2015/cw2013 input unvalid power error\n");
            #endif
        }
        
    }
    if (i >=30){
        reg_val = MODE_SLEEP;
        ret = cw_write(cw_bat->client, REG_MODE, &reg_val);
        #ifdef FG_CW2015_DEBUG
        FG_CW2015_ERR("cw2015/cw2013 input unvalid power error_2\n");
        #endif
        return -1;
    } 
    return 0;
}
