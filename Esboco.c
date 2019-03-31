

if(FD_ISSET(user->fd_tcp_mont,&rfds) && user->state != out)     //Fonte/Acima	
{
	if(user->state==access_server)//Caso seja o root, recebe da fonte em formato fora do protocolo
			{
				if(handle_SOURCEmessage(buffer,user)==0){
					printf("Problems with the source");
					return 0;
				}
			}

		if((n=read(user->fd_tcp_mont,buffer[bytes_avanco],128))!=0)
		{
			if(packet_left > 0)
			{
				nbytes=handle_PACKETmessage(buffer,packet,user,&packet_left,&packet_total,n);
				if(nbytes=1) //O pacote ainda não foi todo recebido.
				{
					nbytes_avanco=o;
					n=0;
				}
				if(nbytes>1)
				{
					shift_left_buffer(buffer,nbytes,n);
					n=n-nbytes;
				}
			}
				
			while(n >0)
			{
				
				if(sscanf(buffer, "%s%n", msgID, &a)==0)
				{
					printf("Failed to read msg_ID\n");
					return 0;
				}
				
				if(strcmp(msgID,"DA")==0)
				{	
					if(n<7)
					{
						bytes_avanco=n;
						break;
					}
					shift_left_buffer(buffer,nbytes,3);
					sscanf(ptr,"%X",&packet_total);
					packet_left=packet_total;
					shift_left_buffer(buffer,nbytes,4);
					nbytes=handle_PACKETmessage(buffer,packet,user,&packet_left,&packet_total,n);
					if(nbytes=1) //O pacote ainda não foi todo recebido.
					{
						nbytes_avanco=o;
						n=0;
					}
					if(nbytes>1)
					{
						shift_left_buffer(buffer,nbytes,n);
						n=n-nbytes;
					}
				
			
			int str_to_msgID(char *ptr, char *msgID)
			{
			  int n = 0, ncount = 0;

			  if(sscanf(ptr, "%s%n", msgID, &n)==1)
			  {
				ptr += n; /* advance the pointer by the number of characters read */
				ncount += n;
				if ((*ptr != ' ')&&(*ptr != '\n')&&(*ptr != '\0'))
				{
				  printf("Incompatible with protocol\n");
				  return 0;
				}
				ncount++;
			  }else{
				printf("Failed to read msg_ID\n");
				return 0;
			  }
			  return ncount;
			}
						
			
			
			
				
				
					
					
			}
					
			   
			printf("%s\n", buffer);
			if(n==-1){printf("error: read\n"); clean_exit(user); exit(1);}
			if(handle_PEERmessage(buffer,user) == 0){printf("Unable to process PEER message\n"); clean_exit(user); exit(1);}
		  }else{
			//Montante saiu
			close(user->fd_tcp_mont);
			user->state = out;
			dissipate("BS\n",user);
		  }
}
 
