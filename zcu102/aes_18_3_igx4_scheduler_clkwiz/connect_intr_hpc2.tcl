connect_bd_net [get_bd_pins sds_irq_const/dout] [get_bd_pins xlconcat_2/In4]
connect_bd_net [get_bd_pins sds_irq_const/dout] [get_bd_pins xlconcat_2/In5]
delete_bd_objs [get_bd_nets xlconstant_0_dout]
delete_bd_objs [get_bd_cells xlconstant_0]
connect_bd_net [get_bd_pins aes_enc1_hpc_1/ap_done] [get_bd_pins aes_enc1_hpc_1_if/ap_done]
connect_bd_net [get_bd_pins aes_enc1_hpc_1/ap_done] [get_bd_pins interrupt_generator_0/core_done]
connect_bd_net [get_bd_pins aes_enc1_hpc_1_if/ap_start] [get_bd_pins aes_enc1_hpc_1/ap_start]
connect_bd_net [get_bd_pins aes_enc1_hpc_1_if/ap_idle] [get_bd_pins aes_enc1_hpc_1/ap_idle]
connect_bd_net [get_bd_pins aes_enc1_hpc_1_if/ap_ready] [get_bd_pins aes_enc1_hpc_1/ap_ready]
connect_bd_net [get_bd_pins aes_enc2_hpc_1/ap_done] [get_bd_pins aes_enc2_hpc_1_if/ap_done]
connect_bd_net [get_bd_pins aes_enc2_hpc_1/ap_done] [get_bd_pins interrupt_generator_1/core_done]
connect_bd_net [get_bd_pins aes_enc2_hpc_1_if/ap_start] [get_bd_pins aes_enc2_hpc_1/ap_start]
connect_bd_net [get_bd_pins aes_enc2_hpc_1_if/ap_idle] [get_bd_pins aes_enc2_hpc_1/ap_idle]
connect_bd_net [get_bd_pins aes_enc2_hpc_1_if/ap_ready] [get_bd_pins aes_enc2_hpc_1/ap_ready]
save_bd_design