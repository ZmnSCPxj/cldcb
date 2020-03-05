#include<deque>
#include<errno.h>
#include<queue>
#include<string.h>
#include<unistd.h>
#include"Protocol/Message.hpp"
#include"ServerTalker/Messenger.hpp"
#include"ServerTalker/rw.hpp"
#include"Util/Logger.hpp"
#include"Util/make_unique.hpp"

namespace ServerTalker {

std::unique_ptr<Protocol::Message>
Messenger::receive_message() {
	/* Read length.  */
	auto l_ciphertext = std::vector<std::uint8_t>(Noise::Encryptor::l_ciphertext_size);
	auto lsize = l_ciphertext.size();
	auto lres = read_all(fd.get(), &l_ciphertext[0], lsize);
	if (!lres) {
		auto my_errno = errno;
		logger.unusual( "While reading message header on <fd %d>: %s"
			      , fd.get()
			      , strerror(my_errno)
			      );
		return nullptr;
	}
	/* Descrypt length.  */
	auto plength = enc.decrypt_length(std::move(l_ciphertext));
	if (!plength) {
		logger.unusual("Failed to decrypt length on <fd %d>.");
		return nullptr;
	}
	auto length = *plength;

	/* Read message. */
	auto msize = Noise::Encryptor::m_ciphertext_additional_size
		   + std::size_t(length)
		   ;
	auto m_ciphertext = std::vector<std::uint8_t>(msize);
	auto mres = read_all(fd.get(), &m_ciphertext[0], msize);
	if (!mres) {
		auto my_errno = errno;
		logger.unusual( "While reading message on <fd %d>: %s"
			      , fd.get()
			      , strerror(my_errno)
			      );
		return nullptr;
	}
	/* Descrypt message.  */
	auto plaintext = enc.decrypt_message(std::move(m_ciphertext));
	if (!plaintext) {
		logger.unusual("Failed to decrypt message on <fd %d>.");
		return nullptr;
	}

	/* Prepare to desriealize message.  */
	auto plaintextd = std::deque<std::uint8_t>( plaintext->begin()
						  , plaintext->end()
						  );
	plaintext = nullptr;
	auto plaintextq = std::queue<std::uint8_t>(std::move(plaintextd));

	/* Deserialize message.  */
	auto ret = Util::make_unique<Protocol::Message>();
	try {
		S::deserialize(plaintextq, *ret);
	} catch (S::InvalidByte const&) {
		logger.unusual( "Invalid byte in incoming message <fd %d>"
			      , fd.get()
			      );
		return nullptr;
	} catch (S::DeserializationTruncated const&) {
		logger.unusual( "Incoming message has incorrect length <fd %d>"
			      , fd.get()
			      );
		return nullptr;
	}
	return ret;
}

bool Messenger::send_message(Protocol::Message message) {
	/* Serialize.  */
	auto plaintext = std::vector<std::uint8_t>();
	S::serialize(plaintext, message);

	/* Encrypt.  */
	auto ciphertexts = enc.encrypt_message(plaintext);

	/* Write.  */
	if (!write_all( fd.get()
		      , &ciphertexts.first[0], ciphertexts.first.size()
		      ))
		return false;
	if (!write_all( fd.get()
		      , &ciphertexts.second[0], ciphertexts.second.size()
		      ))
		return false;
	return true;
}

}

