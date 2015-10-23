using System;
using System.Threading;
using System.Threading.Tasks;

//using SXN.Net.Winsock;

namespace SXN.Net
{
	internal static class Program
	{
		#region Private methods

		private static void Main()
		{
			ThreadPool.SetMinThreads(256, 256);

			// 0 initialize server settings
			var serverSettings = new TcpWorkerSettings
			{
				Port = 5001,
				ReceiveBufferLength = 512,
				SendBufferLength = 512,
				ConnectionsBacklogLength = 32768,
				UseFastLoopback = true,
				UseNagleAlgorithm = false
			};

			// 1 initialize server
			TcpWorker server;

			try
			{
				server = new TcpWorker(serverSettings, ServeSocket);
			}
			catch (TcpServerException e)
			{
				Console.WriteLine($"Error activating server. Kernel error code: {e.KErrorCode}. Winsock error code: {e.WErrorCode}.");

				//Console.WriteLine($"Error activating server.");

				return;
			}

			Console.WriteLine($"server is activated");

			/*
			// 2 try accept connection
			var tryAccept = server.TryAccept();

			if (!tryAccept.Success)
			{
				Console.WriteLine($"error on accept. Kernel error code: {tryAccept.KernelErrorCode}. Winsock error code: {tryAccept.WinsockErrorCode}");
			}

			Console.WriteLine($"accept success");
			*/

			Console.WriteLine("any key to exit");

			Console.ReadLine();

			Console.ReadLine();
		}


		private static async Task ServeSocket(ConnectionHandle connection)
		{
			//Console.WriteLine("Program::ServeSocket[{0:D5}] accepted", connection.Id);

			// asynchronously receive data
			var receiveResult = await connection.ReceiveAsync();

			// await Task.WaitAny(receiveTask, Task.Delay(2000));

			//Console.WriteLine("Program::ServeSocket[{0:D5}] received: {1} bytes", connection.Id, receiveResult);

			// asynchronously receive data
			var x = await connection.SendAsync();

			//Console.WriteLine("Program::ServeSocket[{0:D5}] sent: {1} bytes", connection.Id, x);

			// disconnect
			connection.Disconnect();
		}

		#endregion
	}
}