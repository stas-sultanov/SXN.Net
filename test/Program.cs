namespace SXN.Net;

using System;
using System.Net;
using System.Threading;
using System.Threading.Tasks;

internal static class Program
{
	#region Private methods

	private static void Main()
	{
		var acceptPoint = new IPEndPoint(IPAddress.Loopback, 5001);

		_ = ThreadPool.SetMinThreads(256, 256);

		// 0 initialize server settings
		var serverSettings = new TcpWorkerSettings
		{
			AcceptPoint = acceptPoint,
			AcceptQueueMaxEntriesCount = 1024,
			AcceptQueueWaitTime = TimeSpan.FromMilliseconds(2),
			ReceiveBufferLength = 512,
			SendBufferLength = 512,
			ConnectionsBacklogLength = 4096,
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
			Console.WriteLine($"Error activating server. Kernel error code: {e.KErrorCode}. WinSock error code: {e.WErrorCode}.");

			//Console.WriteLine($"Error activating server.");

			return;
		}

		Console.WriteLine($"server is listening on {acceptPoint}");

		/*
		// 2 try accept connection
		var tryAccept = server.TryAccept();

		if (!tryAccept.Success)
		{
			Console.WriteLine($"error on accept. Kernel error code: {tryAccept.KernelErrorCode}. WinSock error code: {tryAccept.WinSockErrorCode}");
		}

		Console.WriteLine($"accept success");
		*/

		Console.WriteLine("any key to exit");

		_ = Console.ReadLine();

		_ = Console.ReadLine();
	}


	private static async Task ServeSocket(Connection connection)
	{
		Console.WriteLine("Program::ServeSocket[{0:D5}] accepted", connection.Id);

		// asynchronously receive data
		var receiveResult = await connection.ReceiveAsync();

		Console.WriteLine("Program::ServeSocket[{0:D5}] received: {1} bytes", connection.Id, receiveResult);

		// asynchronously receive data
		var x = await connection.SendAsync();

		Console.WriteLine("Program::ServeSocket[{0:D5}] sent: {1} bytes", connection.Id, x);

		// disconnect
		connection.Disconnect();
	}

	#endregion
}
