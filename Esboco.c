

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
					bytes_avanco=o;
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
					shift_left_buffer(buffer,3,n);
					sscanf(ptr,"%X",&packet_total);
					packet_left=packet_total;
					shift_left_buffer(buffer,nbytes,4);
					nbytes=handle_PACKETmessage(buffer,packet,user,&packet_left,&packet_total,n);
					if(nbytes=1) //O pacote ainda não foi todo recebido.
					{
						bytes_avanco=o;
						n=0;
					}
					if(nbytes>1)
					{
						shift_left_buffer(buffer,nbytes,n);
						n=n-nbytes;
					}
				}
				ptr = msg; if((ncount = str_to_msgID(ptr,msgID)) == 0){ //int ncount;
					bytes_avanco=ncount;
					break;
				}
				memcpy(aux,msg,ncount); //char *aux;	
				shift_left_buffer(buffer,ncount,n);
				n=n-ncount;
				if(handle_PEERmessage(aux,user) == 0){printf("Unable to process PEER message\n"); clean_exit(user); exit(1);}		
			}
					
		  }else{
			//Montante saiu
			close(user->fd_tcp_mont);
			user->state = out;
			dissipate("BS\n",user);
		  }
}
 

int shift_left_buffer(char *buffer,int nbytes,int n)
{
	
	/* shifting array elements */
  
    for(i=0;i<n-nbytes-1;i++)
    {
        a[i]=a[nbytes+i];
    }
    a[n-nbytes-1] = { '\0' };
	
}
 
