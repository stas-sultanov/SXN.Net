using System;
using SXN.Net;

namespace Test
{
	internal class Program
	{
		#region Private methods

		private static void Main(String[] args)
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

			// 1 try start server
			var tryStartResultCode = server.Activate();

			if (tryStartResultCode != WinsockErrorCode.None)
			{
				Console.WriteLine($"error starting server:: {tryStartResultCode}");
			}

			// 2 try accept request
			var tryAccept = server.TryAccept();

			if (!tryAccept.Success)
			{
				Console.WriteLine($"error accepting request :: {tryInitialize.ErrorCode}");
			}

			// 3 try stop
		}

		#endregion
	}
}