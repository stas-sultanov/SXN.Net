#include "stdafx.h"

#include "WinsockErrorCode.h"

#include "TcpWorkerSettings.h"

#include "TcpWorker.h"

#pragma unmanaged

void DoWork(SXN::Net::WinsockEx& winsockEx, HANDLE completionPort, RIO_CQ rioReciveCompletionQueue, RIO_CQ rioSendCompletionQueue, INT msgLen)
{
	// define array of completion entries
	OVERLAPPED_ENTRY completionPortEntries[1024];

	// define array of the Registered IO results
	RIORESULT rioResults[1024];

	int rioRecieveNotify = winsockEx.RIONotify(rioReciveCompletionQueue);

	/*
	if (rioRecieveNotify != ERROR_SUCCESS)
	{
	System::Console::WriteLine("RIONotify:recive:{0}", (WinsockErrorCode) rioRecieveNotify);
	}*/

	int rioSendNotify = winsockEx.RIONotify(rioSendCompletionQueue);

	/**
	if (rioSendNotify != ERROR_SUCCESS)
	{
	System::Console::WriteLine("RIONotify:send:{0}", (WinsockErrorCode)rioSendNotify);
	}
	/**/

	// will contain number of entries removed from the completion queue
	ULONG numEntriesRemoved;

	while (true)
	{
		// dequeue completion status
		BOOL dequeueResult = ::GetQueuedCompletionStatusEx(completionPort, completionPortEntries, 1024, &numEntriesRemoved, WSA_INFINITE, FALSE);

		// check if operation has failed
		if (dequeueResult == FALSE)
		{
			continue;
			// TODO: ABORT
		}

		for (unsigned int entryIndex = 0; entryIndex < numEntriesRemoved; entryIndex++)
		{
			// get entry
			OVERLAPPED_ENTRY entry = completionPortEntries[entryIndex];

			switch (entry.lpCompletionKey)
			{
				case SOCK_ACTION_ACCEPT:
				{
					// get overlapped
					SXN::Net::Ovelapped* overlapped = (SXN::Net::Ovelapped *)entry.lpOverlapped;

					// end accept
					int endAcceptResult = overlapped->connection->EndAccepet();

					// check if operation has failed
					if (endAcceptResult == SOCKET_ERROR)
					{
						// get error code
						int winsockErrorCode = ::WSAGetLastError();

						return;
					}

					// start asynchronous receive operation
					overlapped->connection->StartRecieve();

					//System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - ACCEPT", Id, (Int32)overlapped->connectionId);

					break;
				}

				case SOCK_ACTION_DISCONNECT:
				{
					// get overlapped
					SXN::Net::Ovelapped* overlapped = (SXN::Net::Ovelapped *)entry.lpOverlapped;

					// start asynchronous accept operation
					overlapped->connection->StartAccept();

					//System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - DISCONNECT", Id, over2->connectionId);

					break;
				}

				case SOCK_ACTION_RECEIVE:
				{
					// dequeue Registered IO completion results
					ULONG receiveCompletionsCount;
					
					while ((receiveCompletionsCount = winsockEx.RIODequeueCompletion(rioReciveCompletionQueue, rioResults, 1024)) > 0)
					{
						for (int resultIndex = 0; resultIndex < receiveCompletionsCount; resultIndex++)
						{
							// get result
							RIORESULT result = rioResults[resultIndex];

							// get connection
							SXN::Net::TcpConnection* connection = (SXN::Net::TcpConnection *)result.RequestContext;

							// check if client has requested the disconnect by sending 0 bytes
							if (result.BytesTransferred == 0)
							{
								// post disconnect operation
								connection->StartDisconnect();

								continue;
							}

							// start asynchronous send operation
							connection->StartSend(msgLen);

							//System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - RIO RECEIVE, BytesTransferred {2}, SocketContext {3}, Status {4}", Id, result.RequestContext, result.BytesTransferred, result.SocketContext, (WinsockErrorCode) result.Status);
						}
					}

					winsockEx.RIONotify(rioReciveCompletionQueue);

					break;
				}

				case SOCK_ACTION_SEND:
				{
					// dequeue Registered IO completion results
					ULONG sendCompletionsCount;

					while ((sendCompletionsCount = winsockEx.RIODequeueCompletion(rioSendCompletionQueue, rioResults, 1024)) > 0)
					{
						for (ULONG resultIndex = 0; resultIndex < sendCompletionsCount; resultIndex++)
						{
							// get result
							RIORESULT result = rioResults[resultIndex];

							// get connection
							SXN::Net::TcpConnection* connection = (SXN::Net::TcpConnection *) result.RequestContext;

							// start asynchronous disconnect operation
							connection->StartDisconnect();

							//System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - RIO SEND, BytesTransferred {2}, SocketContext {3}, Status {4}", Id, result.RequestContext, result.BytesTransferred, result.SocketContext, (WinsockErrorCode) result.Status);
						}
					}

					winsockEx.RIONotify(rioSendCompletionQueue);

					break;
				}

				default:
				{
					//System::Console::WriteLine("IOCP Thread: {0} something other, iocp_port: {1} num_bytes: {2} key: {3}", Id, (Int32)completionPort, (Int32)numberOfBytes, (Int32)completionKey);

					break;
				}
			}
		}
	}
}

#pragma managed