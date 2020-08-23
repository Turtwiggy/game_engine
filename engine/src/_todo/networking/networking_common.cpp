//#include "networking_common.hpp"
//
///////////////////////////////////////////////////////////////////////////////
////
//// Non-blocking console user input.  Sort of.
//// Why is this so hard?
////
///////////////////////////////////////////////////////////////////////////////
//
//void LocalUserInput_Init()
//{
//	s_pThreadUserInput = new std::thread([&]()
//	{
//		while (!quit)
//		{
//			char szLine[4000];
//			if (!fgets(szLine, sizeof(szLine), stdin))
//			{
//				// Well, you would hope that you could close the handle
//				// from the other thread to trigger this.  Nope.
//				if (quit)
//					return;
//				quit = true;
//				Printf("Failed to read on stdin, quitting\n");
//				break;
//			}
//
//			mutexUserInputQueue.lock();
//			std::string input = std::string(szLine);
//			queueUserInput.push(input);
//			mutexUserInputQueue.unlock();
//		}
//	});
//}
//
//void LocalUserInput_Kill()
//{
//	// Does not work.  We won't clean up, we'll just nuke the process.
//	//	g_bQuit = true;
//	//	_close( fileno( stdin ) );
//	//
//	//	if ( s_pThreadUserInput )
//	//	{
//	//		s_pThreadUserInput->join();
//	//		delete s_pThreadUserInput;
//	//		s_pThreadUserInput = nullptr;
//	//	}
//}
//
//// You really gotta wonder what kind of pedantic garbage was
//// going through the minds of people who designed std::string
//// that they decided not to include trim.
//// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
//
//// Read the next line of input from stdin, if anything is available.
//bool LocalUserInput_GetNext(std::string& result)
//{
//	bool got_input = false;
//	mutexUserInputQueue.lock();
//	while (!queueUserInput.empty() && !got_input)
//	{
//		result = queueUserInput.front();
//		queueUserInput.pop();
//		ltrim(result);
//		rtrim(result);
//		got_input = !result.empty(); // ignore blank lines
//	}
//	mutexUserInputQueue.unlock();
//	return got_input;
//}
