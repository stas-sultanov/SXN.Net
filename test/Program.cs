using System;

namespace SXN.Net
{
	internal static class Program
	{
		#region Private methods

		private static void Main()
		{
			// 0 initialize server settings
			var serverSettings = new TcpServerSettings
			{
				AcceptBacklogLength = TcpServerSettings.MaxConnections,
				Port = 11223,
				UseFastLoopback = true,
				UseNagleAlgorithm = false
			};

			// 1 initialize server
			var tryInitalizeServerResult = RIOSocketServer.TryInitialize(serverSettings);

			if (tryInitalizeServerResult.Success == false)
			{
				Console.WriteLine($"Error activating server. Kernel error code: {tryInitalizeServerResult.KernelErrorCode}. Winsock error code: {tryInitalizeServerResult.WinsockErrorCode}.");

				return;
			}

			var server = tryInitalizeServerResult.Result;

			Console.WriteLine($"server is activated");

			/*
			// 2 try accept connection
			var tryAccept = server.TryAccept();

			if (!tryAccept.Success)
			{
				Console.WriteLine($"error accepting request :: {tryAccept.ErrorCode}");
			}

			Console.WriteLine($"accept success");

			// 3 try deactivate server
			var tryDeactivate = server.Deactivate();

			if (tryDeactivate != ErrorCode.None)
			{
				Console.WriteLine($"error activating server:: {tryStartResultCode}");
			}

			Console.WriteLine($"server is deactivated");
			*/

			Console.ReadLine();
		}

		#endregion
	}
}