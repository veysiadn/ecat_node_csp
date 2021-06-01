#include "ecat_node.hpp"

using namespace EthercatCommunication ; 

/*****************************************************************************************/
/// Extern global variable declaration.
ec_master_t        * g_master = NULL ;           // EtherCAT master instance
ec_master_state_t    g_master_state = {};        // EtherCAT master state
ec_domain_t        * g_master_domain = NULL;     // Ethercat data passing master domain
ec_domain_state_t    g_master_domain_state = {}; // EtherCAT master domain state
struct timespec      g_sync_timer ;
uint32_t             g_sync_ref_counter = 0;
/*****************************************************************************************/

EthercatNode::EthercatNode()
{

}

EthercatNode::~EthercatNode()
{

}

int  EthercatNode::ConfigureMaster()
{
    g_master = ecrt_request_master(0);    
    if (!g_master) {
        
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Requesting master instance failed ! ");
        return -1 ;
    }

    g_master_domain = ecrt_master_create_domain(g_master);
    if(!g_master_domain) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Failed to create master domain ! ");
        return -1 ;
    }
    return 0 ;
}

void EthercatNode::SetCustomSlave(EthercatSlave c_slave, int position)
{
    slaves_[position] = c_slave ; 
}

void EthercatNode::GetAllSlaveInformation()
{
    for(int i=0;i < NUM_OF_SLAVES ; i++){
        ecrt_master_get_slave(g_master, i , &slaves_[i].slave_info_);
    }
}

int  EthercatNode::ConfigureSlaves()
{
    for(int i = 0 ; i < NUM_OF_SLAVES ; i++ ){
        slaves_[i].slave_config_ = ecrt_master_slave_config(g_master,slaves_[i].slave_info_.alias,
                                                                     slaves_[i].slave_info_.position,
                                                                     slaves_[i].slave_info_.vendor_id,
                                                                     slaves_[i].slave_info_.product_code); 
        if(!slaves_[i].slave_config_) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Failed to  configure slave ! ");
            return -1;
        }
    }

    return 0 ;
}

int EthercatNode::SetProfilePositionParameters(ProfilePosParam& P, int position)
{   
  // Operation mode to ProfilePositionMode for slave on that position.
    if( ecrt_slave_config_sdo8(slaves_[position].slave_config_,OD_OPERATION_MODE, kProfilePosition) ){
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set operation mode config error ! ");
        return  -1 ;
    }
    //profile velocity
    if(ecrt_slave_config_sdo32(slaves_[position].slave_config_,OD_PROFILE_VELOCITY, P.profile_vel) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile velocity config error ! ");
        return -1;
    }
    //max profile velocity
    if(ecrt_slave_config_sdo32(slaves_[position].slave_config_,OD_MAX_PROFILE_VELOCITY,P.max_profile_vel) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set max profile velocity config error !");
        return -1;
    }
    //profile acceleration
    if(ecrt_slave_config_sdo32(slaves_[position].slave_config_,OD_PROFILE_ACCELERATION, P.profile_acc) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile acceleration failed ! ");
        return -1;
    }
    //profile deceleration
    if(ecrt_slave_config_sdo32(slaves_[position].slave_config_,OD_PROFILE_DECELERATION,P.profile_dec) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile deceleration failed ! ");
        return -1;
    }
    // quick stop deceleration 
    if(ecrt_slave_config_sdo32(slaves_[position].slave_config_,OD_QUICK_STOP_DECELERATION,P.quick_stop_dec) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set quick stop deceleration failed !");
        return -1;
    }
    // max following error 
    if(ecrt_slave_config_sdo32(slaves_[position].slave_config_,OD_MAX_FOLLOWING_ERROR,P.max_fol_err) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set max following error failed ! ");
        return -1;
    }   
    return 0;
}

int EthercatNode::SetProfilePositionParametersAll(ProfilePosParam& P)
{
    for(int i = 0 ; i < g_kNumberOfServoDrivers ; i++)
    {
        // Set operation mode to ProfilePositionMode for all motors.
        if( ecrt_slave_config_sdo8(slaves_[i].slave_config_,OD_OPERATION_MODE, kProfilePosition) ){
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set operation mode config error ! ");
            return  -1 ;
        }
        //profile velocity
        if(ecrt_slave_config_sdo32(slaves_[i].slave_config_,OD_PROFILE_VELOCITY, P.profile_vel) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile velocity failed ! ");
            return -1;
        }
        //max profile velocity
        if(ecrt_slave_config_sdo32(slaves_[i].slave_config_,OD_MAX_PROFILE_VELOCITY,P.max_profile_vel) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set max profile velocity failed ! ");
            return -1;
        }
        //profile acceleration
        if(ecrt_slave_config_sdo32(slaves_[i].slave_config_,OD_PROFILE_ACCELERATION, P.profile_acc) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile acceleration failed ! ");
            return -1;
        }
        //profile deceleration
        if(ecrt_slave_config_sdo32(slaves_[i].slave_config_,OD_PROFILE_DECELERATION,P.profile_dec) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile deceleration failed ! ");
            return -1;
        }
        // quick stop deceleration 
        if(ecrt_slave_config_sdo32(slaves_[i].slave_config_,OD_QUICK_STOP_DECELERATION,P.quick_stop_dec) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set quick stop deceleration failed !");
            return -1;
        }
        // max following error 
        if(ecrt_slave_config_sdo32(slaves_[i].slave_config_,OD_MAX_FOLLOWING_ERROR,P.max_fol_err) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set max foloowing error failed ! ");
            return -1;
        }   
    }
    return 0; 
}

int EthercatNode::SetProfileVelocityParameters(ProfileVelocityParam& P, int position)
{
    // Set operation mode to ProfileVelocityMode for slave on that position.
    if( ecrt_slave_config_sdo8(slaves_[position].slave_config_,OD_OPERATION_MODE, kProfileVelocity) ){
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set operation mode config error ! ");
        return  -1 ;
    }
    // motionProfileType
    if(ecrt_slave_config_sdo16(slaves_[position].slave_config_,OD_MOTION_PROFILE_TYPE, P.motion_profile_type) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile velocity config error ! ");
        return -1;
    }
    //max profile velocity
    if(ecrt_slave_config_sdo32(slaves_[position].slave_config_,OD_MAX_PROFILE_VELOCITY,P.max_profile_vel) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set max profile  velocity config error ! ");
        return -1;
    }
    //profile acceleration
    if(ecrt_slave_config_sdo32(slaves_[position].slave_config_,OD_PROFILE_DECELERATION, P.profile_dec) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile deceleration failed !");
        return -1;
    }
    //profile deceleration
    if(ecrt_slave_config_sdo32(slaves_[position].slave_config_,OD_PROFILE_ACCELERATION,P.profile_acc) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile acceleration failed ! ");
        return -1;
    }
    // quick stop deceleration 
    if(ecrt_slave_config_sdo32(slaves_[position].slave_config_,OD_QUICK_STOP_DECELERATION,P.quick_stop_dec) < 0) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set quick stop deceleration failed ! ");
        return -1;
    }
    return 0 ;
}

int EthercatNode::SetProfileVelocityParametersAll(ProfileVelocityParam& P)
{
    for(int i = 0 ; i < g_kNumberOfServoDrivers ; i++){
        // Set operation mode to ProfileVelocityMode for all motors.
        if( ecrt_slave_config_sdo8(slaves_[i].slave_config_,OD_OPERATION_MODE, kProfileVelocity) ){
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set operation mode config error ! ");
            return  -1 ;
        }
        // motionProfileType
        if(ecrt_slave_config_sdo16(slaves_[i].slave_config_,OD_MOTION_PROFILE_TYPE, P.motion_profile_type) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile velocity config error ! ");
            return -1;
        }
        //max profile velocity
        if(ecrt_slave_config_sdo32(slaves_[i].slave_config_,OD_MAX_PROFILE_VELOCITY,P.max_profile_vel) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set max profile  velocity config error ! ");
            return -1;
        }
        //profile acceleration
        if(ecrt_slave_config_sdo32(slaves_[i].slave_config_,OD_PROFILE_DECELERATION, P.profile_dec) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile deceleration failed !");
            return -1;
        }
        //profile deceleration
        if(ecrt_slave_config_sdo32(slaves_[i].slave_config_,OD_PROFILE_ACCELERATION,P.profile_acc) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set profile acceleration failed ! ");
            return -1;
        }
        // quick stop deceleration 
        if(ecrt_slave_config_sdo32(slaves_[i].slave_config_,OD_QUICK_STOP_DECELERATION,P.quick_stop_dec) < 0) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Set quick stop deceleration failed ! ");
            return -1;
        }        
    }
    return 0;
}

int EthercatNode::MapDefaultPdos()
{
   /**
    *  This part is specific for our Custom EASYCAT slave configuration
    *  To create your custom slave and variables you can add variables to \see OffsetPDO struct.
    *  Also you have add your variables to received data structure, you may have to create your custom msg files as well.
    **/
    ec_pdo_entry_info_t elmo_pdo_entries[8] = {
        {OD_TARGET_POSITION, 32},
        {OD_DIGITAL_OUTPUTS, 32},
        {OD_CONTROL_WORD, 16},
        {OD_TARGET_VELOCITY,32},
        {OD_POSITION_ACTUAL_VAL, 32},
        {OD_DIGITAL_INPUTS, 32},
        {OD_STATUS_WORD, 16},
        {OD_VELOCITY_ACTUAL_VALUE,32}

    };

    ec_pdo_info_t elmo_pdos[4] = {
        {0x1600, 3, elmo_pdo_entries + 0},
        {0x1607, 1, elmo_pdo_entries + 3},
        
        {0x1a00, 3, elmo_pdo_entries + 4},
        {0x1a07, 1, elmo_pdo_entries + 7}
    };

    ec_sync_info_t elmo_syncs[5] = {
        {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
        {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
        {2, EC_DIR_OUTPUT, 2, elmo_pdos + 0, EC_WD_ENABLE},
        {3, EC_DIR_INPUT, 2, elmo_pdos + 2, EC_WD_DISABLE},
        {0xff}
    };
/*********************************************************/
    ec_pdo_entry_info_t easycat_pdo_entries[16] = {
    {0x0005, 0x01, 16}, /* output_analog_01 */
    {0x0005, 0x02, 16}, /* output_analog_02 */
    {0x0005, 0x03, 16}, /* output_analog_03 */
    {0x0005, 0x04, 8}, /* output_digital_04 */
    {0x0005, 0x05, 8}, /* output_digital_05 */
    {0x0005, 0x06, 8}, /* output_digital_01 */
    {0x0005, 0x07, 8}, /* output_digital_02 */
    {0x0005, 0x08, 8}, /* output_digital_03 */
    {0x0006, 0x01, 16}, /* input_analog_01 */
    {0x0006, 0x02, 16}, /* input_analog_02 */
    {0x0006, 0x03, 16}, /* input_analog_03 */
    {0x0006, 0x04, 8}, /* input_digital_04 */
    {0x0006, 0x05, 8}, /* input_digital_05 */
    {0x0006, 0x06, 8}, /* left_limit_switch */
    {0x0006, 0x07, 8}, /* right_limit_switch */
    {0x0006, 0x08, 8}, /* input_digital_03 */
};

    ec_pdo_info_t easycat_pdos[2] = {
    {0x1600, 8, easycat_pdo_entries + 0}, /* Outputs */
    {0x1a00, 8, easycat_pdo_entries + 8}, /* Inputs */
};

    ec_sync_info_t easycat_syncs[3] = {
    {0, EC_DIR_OUTPUT, 1, easycat_pdos + 0, EC_WD_ENABLE},
    {1, EC_DIR_INPUT, 1, easycat_pdos + 1, EC_WD_DISABLE},
    {0xff}
};

    for(int i = 0 ; i < g_kNumberOfServoDrivers ; i++){
        if(ecrt_slave_config_pdos(slaves_[i].slave_config_,EC_END,elmo_syncs)){
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Slave PDO configuration failed... ");
            return -1;
        }
    }
    
    if(ecrt_slave_config_pdos(slaves_[FINAL_SLAVE].slave_config_,EC_END,easycat_syncs)){
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "EasyCAT slave PDO configuration failed... ");
        return -1;
    }
    
    for(int i = 0; i < g_kNumberOfServoDrivers ; i++){
        this->slaves_[i].offset_.actual_pos        = ecrt_slave_config_reg_pdo_entry(this->slaves_[i].slave_config_,
                                                                                  OD_POSITION_ACTUAL_VAL,g_master_domain,NULL);
        this->slaves_[i].offset_.status_word       = ecrt_slave_config_reg_pdo_entry(this->slaves_[i].slave_config_,
                                                                                  OD_STATUS_WORD,g_master_domain,NULL);
        this->slaves_[i].offset_.actual_vel        = ecrt_slave_config_reg_pdo_entry(this->slaves_[i].slave_config_,
                                                                                  OD_VELOCITY_ACTUAL_VALUE,g_master_domain,NULL);


        this->slaves_[i].offset_.target_pos       = ecrt_slave_config_reg_pdo_entry(this->slaves_[i].slave_config_,
                                                                                  OD_TARGET_POSITION,g_master_domain,NULL);                                                                                                                                                                  
        this->slaves_[i].offset_.target_vel       = ecrt_slave_config_reg_pdo_entry(this->slaves_[i].slave_config_,
                                                                                  OD_TARGET_VELOCITY,g_master_domain,NULL);
        this->slaves_[i].offset_.control_word     = ecrt_slave_config_reg_pdo_entry(this->slaves_[i].slave_config_,
                                                                                  OD_CONTROL_WORD,g_master_domain,NULL);
        if((slaves_[i].offset_.actual_pos < 0)  || (slaves_[i].offset_.status_word  < 0) || (slaves_[i].offset_.actual_vel < 0)
        || (slaves_[i].offset_.target_vel < 0) ||(slaves_[i].offset_.target_pos < 0) || (slaves_[i].offset_.control_word < 0) )
        {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Failed to configure  PDOs for motors.!");
            return -1;
        }
    }

    slaves_[FINAL_SLAVE].offset_.r_limit_switch = ecrt_slave_config_reg_pdo_entry(slaves_[FINAL_SLAVE].slave_config_,
                                                                                  0x006,0x006,g_master_domain,NULL);
    if (slaves_[FINAL_SLAVE].offset_.r_limit_switch < 0){
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"EasyCAT right limit switch PDO configuration failed...\n");
        return -1;
    }
    slaves_[FINAL_SLAVE].offset_.l_limit_switch = ecrt_slave_config_reg_pdo_entry(slaves_[FINAL_SLAVE].slave_config_,
                                                                                  0x006, 0x07, g_master_domain, NULL);
    if (slaves_[FINAL_SLAVE].offset_.l_limit_switch < 0){
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"EasyCAT left limit switch PDO configuration failed...\n");
        return -1;
    }
    slaves_[FINAL_SLAVE].offset_.emergency_switch = ecrt_slave_config_reg_pdo_entry(slaves_[FINAL_SLAVE].slave_config_,
                                                                                  0x006, 0x05, g_master_domain, NULL);
    if (slaves_[FINAL_SLAVE].offset_.emergency_switch < 0){
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"EasyCAT left limit switch PDO configuration failed...\n");
        return -1;
    }
    return 0;
}

int EthercatNode::MapCustomPdos(ec_sync_info_t *syncs, ec_pdo_entry_reg_t *pdo_entry_reg, int position)
{
        int err = ecrt_slave_config_pdos(slaves_[position].slave_config_,EC_END,syncs);
        if ( err ) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Failed to configure  PDOs!  ");
            return -1;
        } 
        err = ecrt_domain_reg_pdo_entry_list(g_master_domain, pdo_entry_reg);
        if ( err ){
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Failed to register PDO entries ");
            return -1;
        }
        return 0;
}

void EthercatNode::ConfigDcSyncDefault()
{
    for(int i=0; i < NUM_OF_SLAVES ; i++){
        ecrt_slave_config_dc(slaves_[i].slave_config_, 0X0300, PERIOD_NS, slaves_[i].kSync0_shift_, 0, 0);
    }
}

void EthercatNode::ConfigDcSync(uint16_t assign_activate, int position)
{
    return ecrt_slave_config_dc(slaves_[position].slave_config_, assign_activate, PERIOD_NS, slaves_[position].kSync0_shift_, 0, 0);
}

void EthercatNode::CheckSlaveConfigurationState()
{
    for(int i = 0 ; i < NUM_OF_SLAVES ;i++)
    {
        slaves_[i].CheckSlaveConfigState();

    }
}

int EthercatNode::CheckMasterState()
{
    ec_master_state_t ms;
    ecrt_master_state(g_master, &ms);
    usleep(10);
    if (ms.slaves_responding != g_master_state.slaves_responding){
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"%u slave(s).\n", ms.slaves_responding);
        if (ms.slaves_responding < 1) {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Connection error,no response from slaves.");
            return -1;
        }
    }
    if (ms.al_states != g_master_state.al_states){
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"AL states: 0x%02X.\n", ms.al_states);
    }
    if (ms.link_up != g_master_state.link_up){
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"Link is %s.\n", ms.link_up ? "up" : "down");
        if(!ms.link_up){ 
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Master state link down");
            return -1;
        }
    }
    g_master_state = ms;
    return 0;
}

void EthercatNode::CheckMasterDomainState()
{
    ec_domain_state_t ds;                     //Domain instance
    ecrt_domain_state(g_master_domain, &ds);
    usleep(10);
    if (ds.working_counter != g_master_domain_state.working_counter)
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"masterDomain: WC %u.\n", ds.working_counter);
    if (ds.wc_state != g_master_domain_state.wc_state)
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"masterDomain: State %u.\n", ds.wc_state);
    if(g_master_domain_state.wc_state == EC_WC_COMPLETE){
        g_master_domain_state = ds;
    }
    g_master_domain_state = ds;
}

int EthercatNode::ActivateMaster()
{   
    if ( ecrt_master_activate(g_master) ) {
        RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Master activation error ! ");
        return -1 ;
    }
    return 0 ; 
}

int EthercatNode::RegisterDomain()
{
    for(int i = 0 ; i < NUM_OF_SLAVES ; i++){
        slaves_[i].slave_pdo_domain_ = ecrt_domain_data(g_master_domain);
        if(!(slaves_[i].slave_pdo_domain_) )
        {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Domain PDO registration error");
            return -1;
        }
    }
    return 0;
}

int EthercatNode::WaitForOperationalMode()
{
    int try_counter=0;
    int check_state_count=0;
    int time_out = 1e4*PERIOD_US;
    while (g_master_state.al_states != EC_AL_STATE_OP ){
        if(try_counter < time_out){
            ecrt_master_receive(g_master);
            ecrt_domain_process(g_master_domain);
            usleep(PERIOD_US);
            if(!check_state_count){
                CheckMasterState();
                CheckMasterDomainState();
                CheckSlaveConfigurationState();
                check_state_count = PERIOD_US ;
            }
            clock_gettime(CLOCK_MONOTONIC, &g_sync_timer);
            ecrt_master_sync_reference_clock_to(g_master, TIMESPEC2NS(g_sync_timer));
            ecrt_master_sync_slave_clocks(g_master);
            ecrt_master_application_time(g_master, TIMESPEC2NS(g_sync_timer));

            ecrt_domain_queue(g_master_domain);                
            ecrt_master_send(g_master);

            try_counter++;
            check_state_count--;
        }else {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Error : Time out occurred while waiting for OP mode.!  ");
            ecrt_master_deactivate_slaves(g_master);
            ecrt_master_deactivate(g_master);
            ecrt_release_master(g_master);
            return -1;
        }
    }
    return 0;
}

int EthercatNode::OpenEthercatMaster()
{
    fd = std::system("ls /dev | grep EtherCAT* > /dev/null");
    if(fd){
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Opening EtherCAT master...");
        std::system("cd ~; sudo ethercatctl start");
        usleep(2e6);
        fd = std::system("ls /dev | grep EtherCAT* > /dev/null");
        if(fd){
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Error : EtherCAT device not found.");
            return -1;
            }else {
                return 0 ;
            }
    }
    return 0 ; 
}

int EthercatNode::GetNumberOfConnectedSlaves()
{
    unsigned int number_of_slaves;
    usleep(1e6);
    ecrt_master_state(g_master,&g_master_state);
    number_of_slaves = g_master_state.slaves_responding ;
    if(NUM_OF_SLAVES != number_of_slaves){
        std::cout << "Please enter correct number of slaves... " << std::endl;
        std::cout << "Entered number of slave : " << NUM_OF_SLAVES << std::endl 
                  << "Connected slaves        : " << number_of_slaves << std::endl;
        return -1; 
    }
    return 0 ;
}

void EthercatNode::DeactivateCommunication()
{
    ecrt_master_deactivate_slaves(g_master);
}

void EthercatNode::ReleaseMaster()
{
    ecrt_master_deactivate(g_master);
    ecrt_release_master(g_master);
}

int EthercatNode::ShutDownEthercatMaster()
{
    fd = std::system("ls /dev | grep EtherCAT* > /dev/null\n");
    if(!fd){
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), " Shutting down EtherCAT master...");
        std::system("cd ~; sudo ethercatctl stop\n");
        usleep(1e6);
        fd = std::system("ls /dev | grep EtherCAT* > /dev/null\n");
        if(fd){
            RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"EtherCAT shut down succesfull.");
            return 0;
        }else {
            RCLCPP_ERROR(rclcpp::get_logger(__PRETTY_FUNCTION__), "Error : EtherCAT shutdown error.");
            return -1 ;
        }
    }
    return 0;
}


