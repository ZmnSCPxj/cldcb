#include<algorithm>
#include<assert.h>
#include<fstream>
#include<sstream>
#include"Archive/RecognitionStorageImpl.hpp"
#include"Daemon/ClientAllow.hpp"
#include"Ev/Io.hpp"
#include"Ev/ThreadPool.hpp"
#include"Util/Logger.hpp"
#include"Util/Str.hpp"

namespace {

std::string stringify_cid(Secp256k1::PubKey const& cid) {
	auto os = std::ostringstream();
	os << cid;
	auto str = os.str();
	str[0] = 'c';
	return str;
}

}

namespace Archive {

void RecognitionStorageImpl::load_codes() {
	auto is = std::ifstream(filename);
	auto count = 0;
	while (is && !is.eof() && !is.bad()) {
		auto ks = std::string();
		is >> ks;
		if (!Util::Str::ishex(ks) || ks.length() != 66) {
			logger.unusual( "Archive::RecognitionStorageImpl: "
					"Invalid key, aborting load: %s"
				      , ks.c_str()
				      );
			return;
		}
		auto vs = std::string();
		if (!Util::Str::ishex(vs) || vs.length() != 128) {
			logger.unusual( "Archive::RecognitionStorageImpl: "
					"Invalid value, aborting load: %s"
				      , vs.c_str()
				      );
			return;
		}
		auto k = Secp256k1::PubKey(ks);
		auto v = Util::Str::hexread(vs);
		codes.emplace(std::move(k), std::move(v));
		++count;
	}
	logger.debug( "Archive::RecognitionStorageImpl: %d codes loaded."
		    , (int) count
		    );
}
bool RecognitionStorageImpl::save_codes() {
	/* TODO: write to a tempfile, then rename afterwards.  */
	auto os = std::ofstream(filename);
	for (auto& e : codes) {
		os << e.first << " "
		   << Util::Str::hexdump(&e.second[0], e.second.size())
		   << std::endl
		    ;
	}
	return true;
}

Ev::Io<bool>
RecognitionStorageImpl::give_recognition_code( Secp256k1::PubKey const& k
					     , std::vector<std::uint8_t> const& v
					     ) {
	return Ev::lift_io(0).then<bool>([this, k, v](int) {
		if (!clientlist.has(k)) {
			logger.unusual( "Client %s is not allowed to store "
					"recognition code, disconnecting."
				      , stringify_cid(k).c_str()
				      );
			return Ev::lift_io(false);
		}
		if (v.size() != 64) {
			logger.unusual( "Client %s tried to give %d-byte "
					"recognition code, disconnecting."
				      , stringify_cid(k).c_str()
				      , (int) v.size()
				      );
			return Ev::lift_io(false);
		}
		return mtx.wait().then<bool>([this, k, v](unsigned int) {
			codes.emplace(k, v);
			return threadpool.background<bool>([this]() {
				return save_codes();
			});
		}).then<bool>([this, k](bool res) {
			return mtx.signal().then<bool>([ this
						       , res
						       , k
						       ](unsigned int) {
				if (res)
					logger.debug( "Client %s updated "
						      "recognition code."
						    , stringify_cid(k).c_str()
						    );
				else
					logger.unusual( "Client %s failed to "
							"update "
							"recognition code."
						      , stringify_cid(k)
								.c_str()
						      );
				return Ev::lift_io(res);
			});
		});
	});
}

Ev::Io<std::vector<std::vector<std::uint8_t>>>
RecognitionStorageImpl::request_recognition_codes() {
	return Ev::lift_io(0)
	     .then<std::vector<std::vector<std::uint8_t>>>([this](int) {
		auto ret = std::vector<std::vector<std::uint8_t>>();

		for (auto& c : codes) {
			assert(c.second.size() == 64);
			auto one = std::vector<std::uint8_t>(97);
			c.first.to_buffer(&one[0]);
			std::copy( c.second.begin(), c.second.end()
				 , one.begin() + 33
				 );
			ret.emplace_back(std::move(one));
		}

		return Ev::lift_io(std::move(ret));
	});
}


}
