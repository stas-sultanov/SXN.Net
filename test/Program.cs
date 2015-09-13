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
				AcceptBacklogLength = 100,
				Port = 11223,
				UseFastLoopback = true,
				UseNagleAlgorithm = false
			};

			// 1 initialize server
			var tryInitalizeServerResult = TcpWorker.TryInitialize(serverSettings);

			if (tryInitalizeServerResult.Success == false)
			{
				Console.WriteLine($"Error activating server. Kernel error code: {tryInitalizeServerResult.KernelErrorCode}. Winsock error code: {tryInitalizeServerResult.WinsockErrorCode}.");

				return;
			}

			var server = tryInitalizeServerResult.Result;

			Console.WriteLine($"server is activated");

			// 2 try accept connection
			var tryAccept = server.TryAccept();

			if (!tryAccept.Success)
			{
				Console.WriteLine($"error on accept. Kernel error code: {tryAccept.KernelErrorCode}. Winsock error code: {tryAccept.WinsockErrorCode}");
			}

			Console.WriteLine($"accept success");

			Console.WriteLine("any key to exit");

			Console.ReadLine();

			// 3 try deactivate server
			var tryDeactivate = server.Deactivate();

			if (tryDeactivate != WinsockErrorCode.None)
			{
				Console.WriteLine($"error activating server:: {tryDeactivate}");
			}

			Console.WriteLine($"server is deactivated");
			

			Console.ReadLine();
		}

		#endregion
	}
}