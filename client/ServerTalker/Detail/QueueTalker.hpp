#ifndef CLDCB_CLIENT_SERVERTALKER_DETAIL_QUEUETALKER_HPP
#define CLDCB_CLIENT_SERVERTALKER_DETAIL_QUEUETALKER_HPP

#include<memory>
#include<mutex>
#include<queue>
#include"Net/Fd.hpp"

namespace ServerTalker { namespace Detail {

/* Handles a queoe of items, but also simultaneously waits
 * for activity on another fd (presumably a network socket
 * fd).
 */
class QueueTalker {
public:
	/* Abstract class used to represent an item.  */
	class Item {
	public:
		virtual ~Item() { }
	};

	/* Result of waiting.  */
	enum ResultType
	/* An item is available.  */
	{ ItemAvailable
	/* The FD has activity.  */
	, FdAvailable
	/* An abort was posted.  */
	, Abort
	};

	/* Actual result.  */
	struct Result {
		ResultType type;
		std::unique_ptr<Item> item;
	};

	/* What to wake up for on fd.  */
	enum WaitType
	{ Read
	, Write
	, ReadWrite
	, Neither
	};

private:
	/* File descriptor to additionally monitor.  */
	int fd;
	/* Signaling pipe, used to wake up the waiter when
	 * an item is available.
	 * The number of bytes in the pipe is effectively
	 * used as a semaphore, one which we can wait for
	 * together with the network file descriptor.
	 * pipe_in is the write end, pipe_out the read end.
	 */
	Net::Fd pipe_in;
	Net::Fd pipe_out;

	std::mutex mtx;
	std::queue<std::unique_ptr<Item>> items;
	bool aborted;

public:
	QueueTalker() =delete;
	QueueTalker(QueueTalker&&) =delete;
	QueueTalker(QueueTalker const&) =delete;

	explicit QueueTalker(int fd_);

	/* Used by waiting thread.  */
	Result wait(WaitType wait_for);

	/* Used by user thread.  */
	/* Argument item must be non-nullptr.  */
	void post(std::unique_ptr<Item> item);
	/* Cause the waiting thread to receive an abort result.  */
	void abort();

private:
	/* Make any waiter return.  */
	void raise_pipe();
};

}}

#endif /* CLDCB_CLIENT_SERVERTALKER_DETAIL_QUEUETALKER_HPP */
