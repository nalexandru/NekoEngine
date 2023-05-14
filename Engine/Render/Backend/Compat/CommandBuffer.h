#ifndef NE_COMMAND_BUFFER
#define NE_COMMAND_BUFFER

/*
 * Command Buffer emulation for APIs that don't support them
 */

#include <Engine/Types.h>

struct NeCommandBuffer;

#ifdef __cplusplus

#include <vector>
#include <type_traits>

union NeCommandArgument
{
	int8_t i8;
	uint8_t u8;
	int16_t i16;
	uint16_t u16;
	int32_t i32;
	uint32_t u32;
	int64_t i64;
	uint64_t u64;
	float flt;
	double dbl;
	bool bln;
	void *ptr;
};

struct NeCommand
{
	void (*exec)(struct NeCommandBuffer *);
	uint32_t argc;
};

struct NeCommandBuffer
{
	std::vector<NeCommand> commands;
	std::vector<NeCommandArgument> args;
	uint32_t argOffset;

	template<typename T>
	void addArg(T arg)
	{
		if (std::is_pointer<T>::value) {
			NeCommandArgument ca;
			ca.ptr = (void *)arg;
			args.push_back(ca);
		} else {
			args.push_back((NeCommandArgument)arg);
		}
	}

	NeCommandArgument &
	arg(int argc) { return args[argOffset + argc]; };

	template<typename T> T
	arg(int argc)	{ return std::is_pointer<T>::value ? (T)args[argOffset + argc].ptr : (T)args[argOffset + argc].u64; }
};

#define CMD_ARG(type, m)														\
template<> type																	\
NeCommandBuffer::arg<type>(int argc) { return args[argOffset + argc].m; }		\
template<> void																	\
NeCommandBuffer::addArg<type>(type arg) {	args.push_back({ .m = arg }); }

CMD_ARG(int8_t, i8)
CMD_ARG(uint8_t, u8)
CMD_ARG(int16_t, i16)
CMD_ARG(uint16_t, u16)
CMD_ARG(int32_t, i32)
CMD_ARG(uint32_t, u32)
CMD_ARG(int64_t, i64)
CMD_ARG(uint64_t, u64)
CMD_ARG(float, flt)
CMD_ARG(double, dbl)
CMD_ARG(bool, bln)
CMD_ARG(void *, ptr)

#undef CMD_ARG

template<typename T>
struct NeCmdArgLoader
{
	struct NeCommand *c;
	struct NeCommandBuffer *cb;
	const T &_obj;

	constexpr void operator()() { cb->addArg(_obj); ++c->argc; };
};

inline bool
InitCommandBuffer(struct NeCommandBuffer *cb)
{
	cb->commands.reserve(100);
	cb->args.reserve(400);

	return true;
}

template<typename... Args> inline void
RecordCommand(struct NeCommandBuffer *cb, void (*func)(struct NeCommandBuffer *), Args &&...args)
{
	NeCommand c{ func, 0 };
	(NeCmdArgLoader<Args>{ &c, cb, args }(), ...);
	cb->commands.push_back(c);
}

inline void
EndCommandBuffer(struct NeCommandBuffer *cb)
{
}

inline void
ExecuteCommands(struct NeCommandBuffer *cb)
{
	for (const NeCommand &c : cb->commands) {
		c.exec(cb);
		cb->argOffset += c.argc;
	}
}

inline void
ResetCommandBuffer(struct NeCommandBuffer *cb)
{
	cb->commands.clear();
	cb->args.clear();
	cb->argOffset = 0;
}

inline void
TermCommandBuffer(struct NeCommandBuffer *cb)
{
}

#endif

#endif /* NE_COMMAND_BUFFER */
