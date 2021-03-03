nw_cli.o: nw_cli.h utils.h nw_err.h
utils.o: utils.h
nw_comm_cmd.o: nw_cli.h utils.h
nw_other_cmd.o: utils.h nw_cli.h
nw_peer_cmd.o: utils.h nw_cli.h nw_err.h
