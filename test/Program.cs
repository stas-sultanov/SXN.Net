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
			var server = new TcpServer(serverSettings);

			// 2 try activate server
			var tryStartResultCode = server.Activate();

			if (tryStartResultCode != WinsockErrorCode.None)
			{
				Console.WriteLine($"error activating server:: {tryStartResultCode}");

				return;
			}

			Console.WriteLine($"server is activated");

			// 2 try accept connection
			var tryAccept = server.TryAccept();

			if (!tryAccept.Success)
			{
				Console.WriteLine($"error accepting request :: {tryAccept.ErrorCode}");
			}

			Console.WriteLine($"accept success");

			// 3 try deactivate server
			var tryDeactivate = server.Deactivate();

			if (tryDeactivate != WinsockErrorCode.None)
			{
				Console.WriteLine($"error activating server:: {tryStartResultCode}");
			}

			Console.WriteLine($"server is deactivated");

			Console.ReadLine();
		}

		#endregion
	}
}