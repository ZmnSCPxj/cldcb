#include<algorithm>
#include<assert.h>
#include<stdexcept>
#include"Daemon/Breaker.hpp"
#include"Daemon/IoResult.hpp"
#include"Daemon/Messenger.hpp"
#include"Ev/Io.hpp"
#include"Noise/Encryptor.hpp"
#include"Protocol/Message.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace Daemon {

Ev::Io<std::unique_ptr<Protocol::Message>>
Messenger::receive_message(double initial_timeout, bool& timedout) {
	return initial_receive(initial_timeout, timedout)
	     .then<std::unique_ptr<Protocol::Message>>(
			[this](std::unique_ptr<std::uint16_t> psize) {
		if (!psize)
			return Ev::lift_io<std::unique_ptr<Protocol::Message>>(nullptr);
		auto msglen = *psize
			    + Noise::Encryptor::m_ciphertext_additional_size
			    ;

		return read_message( std::vector<std::uint8_t>()
				   , msglen
				   );
	});
}

Ev::Io<std::unique_ptr<std::uint16_t>>
Messenger::initial_receive(double initial_timeout, bool& timedout) {
	timedout = false;
	return breaker.read_timed( fd.get()
				 , Noise::Encryptor::l_ciphertext_size
				 , initial_timeout
				 )
	     .then<std::unique_ptr<std::uint16_t>>([ this
						   , &timedout
						   ](IoResult ior) {
		/* These break our connection, so --- */
		if ( ior.result == IoEof
		  || ior.result == IoBroken
		   )
			return Ev::lift_io<std::unique_ptr<std::uint16_t>>(nullptr);

		/* Nothing read?  Just return.  */
		if (ior.data.size() == 0) {
			timedout = true;
			return Ev::lift_io<std::unique_ptr<std::uint16_t>>(nullptr);
		}

		return read_length(std::move(ior.data));
	});
}
Ev::Io<std::unique_ptr<std::uint16_t>>
Messenger::read_length(std::vector<std::uint8_t> data) {
	return breaker.read_timed( fd.get()
				 , Noise::Encryptor::l_ciphertext_size
				 , -1.0
				 , std::move(data)
				 )
	     .then<std::unique_ptr<std::uint16_t>>([this
						   ](IoResult ior) {
		if ( ior.result == IoEof
		  || ior.result == IoBroken
		   )
			return Ev::lift_io<std::unique_ptr<std::uint16_t>>(nullptr);

		if (ior.data.size() == Noise::Encryptor::l_ciphertext_size) {
			auto rv = enc.decrypt_length(ior.data);
			if (!rv)
				logger.unusual( "Failed to decrypt length "
						"from incoming header "
						"<fd %d>"
					      , fd.get()
					      );
			return Ev::lift_io(std::move(rv));
		}

		return read_length(std::move(ior.data));
	});
}
Ev::Io<std::unique_ptr<Protocol::Message>>
Messenger::read_message(std::vector<std::uint8_t> data, std::size_t msglen) {
	return breaker.read_timed( fd.get()
				 , msglen
				 , -1.0
				 , std::move(data)
				 )
	     .then<std::unique_ptr<Protocol::Message>>([ this
						       , msglen
						       ](IoResult ior) {
		if ( ior.result == IoEof
		  || ior.result == IoBroken
		   )
			return Ev::lift_io<std::unique_ptr<Protocol::Message>>(nullptr);

		if (ior.data.size() < msglen)
			return read_message(std::move(ior.data), msglen);

		auto plaintext = enc.decrypt_message(std::move(ior.data));
		if (!plaintext) {
			logger.unusual( "Failed to decrypt incoming message "
				        "<fd %d>"
				      , fd.get()
				      );
			return Ev::lift_io<std::unique_ptr<Protocol::Message>>(nullptr);
		}

		/* Copy the data to a queue, for deserialization.  */
		auto plaintextd = std::deque<std::uint8_t>(plaintext->begin(), plaintext->end());
		plaintext = nullptr;
		auto plaintextq = std::queue<std::uint8_t>(std::move(plaintextd));

		auto msg = Util::make_unique<Protocol::Message>();
		try {
			S::deserialize(plaintextq, *msg);
		} catch (S::InvalidByte const&) {
			logger.unusual( "Invalid byte in incoming message "
				        "<fd %d>"
				      , fd.get()
				      );
			return Ev::lift_io<std::unique_ptr<Protocol::Message>>(nullptr);
		} catch (S::DeserializationTruncated const&) {
			logger.unusual( "Incoming message has "
					"incorrect length "
				        "<fd %d>"
				      , fd.get()
				      );
			return Ev::lift_io<std::unique_ptr<Protocol::Message>>(nullptr);
		}

		return Ev::lift_io(std::move(msg));
	});
}


Ev::Io<bool>
Messenger::send_message(Protocol::Message message) {
	/* Serialize.  */
	auto plaintext = std::vector<std::uint8_t>();
	S::serialize(plaintext, message);
	if (plaintext.size() > 65535)
		throw std::logic_error("Daemon::Messenger::send_message: "
				       "message sent > 65535");

	/* Encrypt.  */
	auto ciphertexts = enc.encrypt_message(std::move(plaintext));
	auto& l_ciphertext = ciphertexts.first;
	auto& m_ciphertext = ciphertexts.second;

	/* Append m_ciphertext to l_ciphertext.  */
	auto l_ciphertext_size = l_ciphertext.size();
	l_ciphertext.resize(l_ciphertext_size + m_ciphertext.size());
	std::copy( m_ciphertext.begin(), m_ciphertext.end()
		 , l_ciphertext.begin() + l_ciphertext_size
		 );
	/* Free up m buffer.  */
	m_ciphertext.clear();

	/* Send.  */
	return breaker.write_timed( fd.get()
				  , std::move(l_ciphertext)
				  ).then<bool>([](IoResult ior) {
		if ( ior.result == IoEof
		  || ior.result == IoBroken
		   )
			return Ev::lift_io(false);
		/* Only other possibility of partial write is timeout,
		 * but we specifically do not allow timeout.
		 */
		assert(ior.data.size() == 0);

		return Ev::lift_io(true);
	});
}

}

