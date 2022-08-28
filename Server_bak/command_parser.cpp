#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iostream"

#include "command_parser.h"
#include "command_define_list.h"
#include "tracex.h"

#define ThisID Server

using namespace std;
int test(HEADERPACKET* msg);
int video_data_send(HEADERPACKET* msg);
int hash_send(HEADERPACKET* msg);
int CID_send(HEADERPACKET* msg);


cmdp_desp parser_desp[] =
{
	// {PUBKEY_SND					, public_key_send		},
	// {PUBKEY_RES					, public_key_response	},
	// {PRIKEY_SND					, private_key_send		},
	// {PRIKEY_RES					, private_key_response	},
	{VIDEO_DATA_SND			, video_data_send		},
	// {VIDEO_DATA_RES				, video_data_response	},
	// {HASH_REQ					, hash_request			},
	// {HASH_SND					, hash_send				},
	// {NEW_HASH_SND				, new_hash_send			},
	// {NEW_HASH_RES				, new_hash_response		},
	// {CID_REQ					, CID_request			},
	// {CID_SND					, CID_send				},
	// {VER_RSLT_SND				, verified_result_send	},
	// {VER_RSLT_RES				, verified_result_response},
	// {CONFIG_REQ					, configuration_request	},
	// {CONFIG_RES					, configuration_response},



	{TEST_CMD					, test					},
};

//-----------------------------------------------------------------------------
//	cmd_parser
//-----------------------------------------------------------------------------
#ifndef _CMD_PARSER_
#define _CMD_PARSER_
int cmd_parser(IO_PORT port, HEADERPACKET *pmsg)
{
	int ack, i=0, len;
	len = sizeof(parser_desp) / sizeof(parser_desp[0]);

	if(pmsg->destID != ThisID)
		return -1; //"wrong send";

	for (i = 0, ack = -3; i < len; i++){
		if(parser_desp[i].code == pmsg->command){
			ack = parser_desp[i].callback(pmsg);
		}
	}
}
#endif

int test(HEADERPACKET* msg){
	cout << "test function" << endl;
	return 1;
}




