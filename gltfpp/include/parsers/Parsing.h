#pragma once
#include "../Error.h"
#include "../Meta.h"
#include <boost/hana.hpp>
#include <boost/hana/define_struct.hpp>
#include <functional>
#include <json.hpp>

namespace gltfpp {
	inline namespace v1 {
		struct glTF;

		struct ParseContext {
			glTF *root = nullptr;
			const nlohmann::json *json = nullptr;
		};

		template <typename T, typename std::enable_if_t<detail::is_field_aggregate<T>>* = nullptr>
		auto parse(T &target);

		template <typename T, typename std::enable_if_t<detail::is_fundamental_json_type<T>>* = nullptr>
		auto parse(T &target);

		template <typename T, typename std::enable_if_t<detail::is_field_list<T>>* = nullptr>
		auto parse(T &target);

		template <typename T>
		auto field(option<T> &target, const char *key) {
			return [&, key](ParseContext ctx) -> gltf_result<ParseContext> {
				auto valIt = ctx.json->find(key);
				if(valIt != ctx.json->end()) {
					target.set_value();
					auto newCtx = ParseContext{ctx.root, std::addressof(*valIt)};
					auto res = parse(target.value())(newCtx);
					static_assert(std::is_same<gltf_result<ParseContext>, decltype(res)>{},
								  "Return type of the parser function must be gltf_result<ParseContext>");
				} else {
					target = {};
				}
				return ctx;
			};
		}

		template <typename T>
		auto field(T &target, const char *key) {
			return [&, key](ParseContext ctx) -> gltf_result<ParseContext> {
				auto valIt = ctx.json->find(key);
				if(valIt != ctx.json->end()) {
					auto newCtx = ParseContext{ctx.root, std::addressof(*valIt)};
					auto res = parse(target)(newCtx);
					static_assert(std::is_same<gltf_result<ParseContext>, decltype(res)>{},
								  "Return type of the parser function must be gltf_result<ParseContext>");
					if(!res) {
						return res;
					}
					return ctx;
				}
				return make_unexpected(gltf_error::key_not_found);
			};
		}

		template <typename T>
		auto aggregate(T &target) {
			return [&](ParseContext ctx) -> gltf_result<ParseContext> {
				constexpr auto accessor = boost::hana::accessors<T>();
				auto names = boost::hana::transform(accessor, boost::hana::first);
				auto members = boost::hana::transform(accessor, boost::hana::second);
				auto refs = boost::hana::transform(members, [&](auto acc) { return std::ref(acc(target)); });

				auto res = boost::hana::fold(
					boost::hana::zip(names, refs), gltf_result<ParseContext>{ctx}, [&](auto c, auto entry) {
						if(!c) {
							return c;	// I hope the optimizer understands that...
						}
						auto name = boost::hana::to<const char *>(entry[boost::hana::size_c<0>]);
						auto &member = entry[boost::hana::size_c<1>].get();
						return c >> field(member, name);
					});
				if(!res) {
					return res;
				}
				return ctx;
			};
		}

		template <typename T, typename std::enable_if_t<detail::is_field_aggregate<T>>*>
		auto parse(T &target) {
			return aggregate(target);
		}

		template <typename T, typename std::enable_if_t<detail::is_fundamental_json_type<T>>*>
		auto parse(T &target) {
			return [&](ParseContext ctx) -> gltf_result<ParseContext> {
				// TODO this is not a complete check
				if(ctx.json) {
					target = ctx.json->template get<T>();
					return ctx;
				}
				return make_unexpected(gltf_error::key_not_found);
			};
		}

		template <typename T, typename std::enable_if_t<detail::is_field_list<T>>*>
		auto parse(T &target) {
			return [&](ParseContext ctx) -> gltf_result<ParseContext> {
				if(!ctx.json) {
					return make_unexpected(gltf_error::key_not_found);
				}
				if(!ctx.json->is_array()) {
					return make_unexpected(gltf_error::type_error);
				}

				target.resize(ctx.json->size());

				auto in = ctx.json;
				auto out = target.begin();
				for(; out != target.end(); ++in, ++out) {
					auto res = parse(*out)({ctx.root, in});
					static_assert(std::is_same<gltf_result<ParseContext>, decltype(res)>{},
								  "Return type of the parser function must be gltf_result<ParseContext>");
					if(!res) {
						return res;
					}
				}
				return ctx;
			};
		}
	}	// namespace v1
}	// namespace gltfpp
