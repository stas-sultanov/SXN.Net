using System;

namespace SXN.Net
{
	internal static class Program
	{
		#region Private methods

		private static void Main()
		{
			// 0 try initialize server
			var tryInitialize = TcpServer.TryInitialize(10202);

			if (!tryInitialize.Success)
			{
				Console.WriteLine($"error initializing server socket:: {tryInitialize.ErrorCode}");

				return;
			}

			var server = tryInitialize.Result;

			Console.WriteLine("init success");

			// 1 try activate server
			var tryStartResultCode = server.Activate();

			if (tryStartResultCode != WinsockErrorCode.None)
			{
				Console.WriteLine($"error activating server:: {tryStartResultCode}");
			}

			// 2 try accept connection
			var tryAccept = server.TryAccept();

			if (!tryAccept.Success)
			{
				Console.WriteLine($"error accepting request :: {tryInitialize.ErrorCode}");
			}

			Console.WriteLine($"accept success");

			// 3 try deactivate server
			var tryDeactivate = server.Deactivate();

			if (tryDeactivate != WinsockErrorCode.None)
			{
				Console.WriteLine($"error activating server:: {tryStartResultCode}");
			}

			Console.ReadLine();
		}

		#endregion
	}
}