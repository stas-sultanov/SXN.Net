using System;
//using SXN.Net.Winsock;

namespace SXN.Net
{
	internal static class Program
	{
		#region Private methods

		private static void Main()
		{
			
			// 0 initialize server settings
			var serverSettings = new TcpWorkerSettings
			{
				Port = 11224,
				ReciveBufferLength = 1024,
				SendBufferLength = 1024,
				ConnectinosBacklogLength = 128,
				UseFastLoopback = true,
				UseNagleAlgorithm = false
			};

			// 1 initialize server
			TcpWorker server;

			try
			{
				server = new TcpWorker(serverSettings);
			}
			catch (TcpServerException e)
			{
				//Console.WriteLine($"Error activating server. Kernel error code: {tryInitalizeServerResult.KernelErrorCode}. Winsock error code: {tryInitalizeServerResult.WinsockErrorCode}.");

				Console.WriteLine($"Error activating server.");

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

		#endregion
	}
}