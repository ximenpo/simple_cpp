#ifndef BUFFER_H_66BC65DB_AFF6_43C8_8654_D1A2801635E2
#define BUFFER_H_66BC65DB_AFF6_43C8_8654_D1A2801635E2

#include <cassert>
#include <iosfwd>

#include <map>
#include <string>

#include "array.h"

#if		!defined(BUFFER_DEFAULT_SIZE)
#	define	BUFFER_DEFAULT_SIZE				4 * 1024
#endif

//
//	序列化缓冲区类(Dirty and easy.)
//
class buffer {
public:
    explicit	buffer(unsigned long nMaxSize = BUFFER_DEFAULT_SIZE);
    explicit	buffer(const void* pBuf, unsigned long nLen);

    ~buffer();

public:
    // 清除缓冲区数据
    void		clear() {
        failure_ = false;
        size_ = 0;
        offset_ = 0;
    }
    // 复位数据区指针
    void		rewind() {
        failure_ = false;
        offset_= 0;
    }

    // 获取数据信息
    const void*	data()const {
        return data_;
    }
    // 获取数据大小
    unsigned long	size()const {
        return size_;
    }
    // 获取数据大小
    unsigned long	capacity()const {
        return capacity_;
    }

    // 获取当前读取偏移
    unsigned long	pos()const {
        return offset_;
    }
    // 获取当前读取的数据
    const void*	data_from_curr_pos()const {
        return &data_[offset_];
    }

public:
    // 判断当前状态
    bool		good()const {
        return !failure_;
    }
    bool		failure()const {
        return failure_;
    }
    void		set_failure() {
        failure_ = true;
    }

public:
    // 写入指定长度字节数
    bool		write(const void* pData, unsigned long nLen);
    // 读出指定长度字节数
    bool		read(void* pData, unsigned long nLen);

public:
    // 将数据包内容以十六进制的方式输出为文本内容
    std::string		dump();
    std::ostream&	dump(std::ostream& os);

private:
    bool				buf_owner_;
    bool				failure_;
    unsigned char*		data_;
    unsigned long		capacity_;
    unsigned long		size_;
    unsigned long		offset_;

private:
    buffer(const buffer& rhs);
    buffer& operator=(const buffer& rhs);
};

//
//	数据序列化类型标记
//
struct	buffer_tag {
    enum	DATA_TYPE {
        TYPE_NONE,			// nothing, dummy data
        TYPE_RAW,			// byte array
        TYPE_BOOL,			// boolean
        TYPE_INT,			// int8, int16, int32
        TYPE_UINT,			// uint8, uint16, uint32
        TYPE_REAL,			// float, double
        TYPE_STRING,		// UTF-8 strings
        TYPE_ARRAY,			// array
        TYPE_OBJECT,		// key/value pairs
        TYPE_SUM,
    };

    enum	SIZE_TAG {
        TAG_0,
        TAG_1,
        TAG_2,
        TAG_4,
        TAG_SUM,
    };

    // data member
    unsigned long	data_type;
    unsigned long	size_tag;
    bool			version_tag;	// 对于结构而言是版本号,对于数组而言是数据项包含的对象数(当前仅支持1or2)
    bool			unused_tag;		// 未使用

    // Serailize/UnSerialize with byte
    unsigned char	pack()const {
        return (unsigned char)((unused_tag?0x80:0x00) | (version_tag?0x40:0x00) | (size_tag << 4) | (data_type << 0));
    }
    void			unpack(unsigned char value) {
        data_type	= (value&0x0F);
        size_tag	= ((value&0x30)>>4);
        version_tag	= (value&0x40) != 0;
        unused_tag	= (value&0x80) != 0;
    }
};

//
// size byte
//
buffer_tag::SIZE_TAG	buffer_size_tag(int value);
buffer_tag::SIZE_TAG	buffer_size_tag(unsigned long value);
buffer_tag::SIZE_TAG	buffer_size_tag(unsigned int value);
buffer_tag::SIZE_TAG	buffer_size_tag(float value);
buffer_tag::SIZE_TAG	buffer_size_tag(double value);
buffer_tag::SIZE_TAG	buffer_size_tag(bool value);

//
//	基本类型序列化
//

//
//	utils.
//
bool		buffer_write_tag(buffer& buf, const buffer_tag& tag);
bool		buffer_read_tag(buffer& buf, buffer_tag& tag);

bool		buffer_write_uint_value(buffer& buf, int size_tag, unsigned long value);
bool		buffer_read_uint_value(buffer& buf, int size_tag, unsigned long& value);

//
//	Raw:	byte array
//
bool		buffer_write_raw(buffer& buf, const void* pData, unsigned long nLen);
bool		buffer_read_raw(buffer& buf, void* pData, unsigned long& nLen);

//
//	None:	ignore
//
bool		buffer_read_and_ignore(buffer& buffer);

//
//	BOOL:	bool
//
buffer&		operator<<(buffer& buf, bool value);
buffer&		operator>>(buffer& buf, bool& value);

//
//	INT:	int/char/short
//
buffer&		operator<<(buffer& buf, signed int value);
buffer&		operator<<(buffer& buf, signed char value);
buffer&		operator<<(buffer& buf, signed short value);
buffer&		operator<<(buffer& buf, signed long value);

buffer&		operator>>(buffer& buf, signed int& value);
buffer&		operator>>(buffer& buf, signed char& value);
buffer&		operator>>(buffer& buf, signed short& value);
buffer&		operator>>(buffer& buf, signed long& value);

//
//	UINT:	int/char/short
//
buffer&		operator<<(buffer& buf, unsigned char value);
buffer&		operator<<(buffer& buf, unsigned short value);
buffer&		operator<<(buffer& buf, unsigned long value);
buffer&		operator<<(buffer& buf, unsigned int value);

buffer&		operator>>(buffer& buf, unsigned char& value);
buffer&		operator>>(buffer& buf, unsigned short& value);
buffer&		operator>>(buffer& buf, unsigned long& value);
buffer&		operator>>(buffer& buf, unsigned int& value);

//
//	REAL:	float/double
//
buffer&		operator<<(buffer& buf, float value);
buffer&		operator>>(buffer& buf, float& value);

buffer&		operator<<(buffer& buf, double value);
buffer&		operator>>(buffer& buf, double& value);

//
//	STRING:	Ansi/Unicode <-> utf-8
//
buffer&		operator<<(buffer& buf, const std::string& value);
buffer&		operator>>(buffer& buf, std::string& value);

#if	!defined(__GNUC__) || defined (_GLIBCXX_USE_WCHAR_T) || defined (_GLIBCXX_USE_WSTRING)
buffer& 	operator<<(buffer& buf, const std::wstring& value);
buffer&		operator>>(buffer& buf, std::wstring& value);
#endif

//
//	Array:	vector/deque/set/list/map/pair
//
template<typename T>
static	unsigned long	buffer_get_array_version_tag(const T& array) {
    return	0;
}
template<class KEY, class VALUE>
static	unsigned long	buffer_get_array_version_tag(const std::map<KEY,VALUE>& array) {
    return	2;
}

template<class T>
inline bool			buffer_read_array_item(buffer& buf, T& array) {
    typename	T::value_type	t;
    buf	>>	t;
    if(buf.failure()) {
        return	false;
    }
    array.insert(array.end(), t);
    return	true;
}

template<class KEY, class VALUE>
inline bool			buffer_read_array_item(buffer& buf, std::map<KEY,VALUE>& array) {
    KEY	k;
    VALUE v;
    buf	>>	k	>> v;
    if(buf.failure()) {
        return	false;
    }
    array.insert(array.end(), std::make_pair(k,v));
    return	true;
}

template<class KEY, class VALUE>
inline buffer&		operator<<(buffer& buf, const std::pair<KEY, VALUE>& obj) {
    return	buf << obj.first << obj.second;
}

template<class KEY, class VALUE>
inline buffer&		operator>>(buffer& buf, std::pair<KEY, VALUE>& obj) {
    return	buf >> obj.first >> obj.second;
}

template<typename T>
inline buffer&		operator<<(buffer& buf, const T& array) {
    unsigned long	version		= buffer_get_array_version_tag(array);
    buffer_tag		tag	= {
        buffer_tag::TYPE_ARRAY,
        buffer_size_tag(array.size()),
        version >= 2
    };

    //	begin tag.
    if(		!buffer_write_tag(buf, tag)
            ||	!buffer_write_uint_value(buf, tag.size_tag, array.size())
      ) {
        buf.set_failure();
        return	buf;
    }

    //	version tag.
    if(tag.version_tag) {
        buffer_write_uint_value(buf, buffer_tag::TAG_1, version);
    }

    //	content.
    unsigned long	size			= array.size();
    typename T::const_iterator it	= array.begin();
    for(unsigned long i = 0; i < size; ++i, ++it) {
        buf	<< (*it);
    }

    return buf;
}

template<typename T>
inline buffer&		operator>>(buffer& buf, T& array) {
    array.clear();

    unsigned long	size;
    unsigned long	version;
    buffer_tag	tag;
    if(		!buffer_read_tag(buf, tag)
            ||	tag.data_type != buffer_tag::TYPE_ARRAY
            ||	!buffer_read_uint_value(buf, tag.size_tag, size)
      ) {
        buf.set_failure();
        return	buf;
    }

    //	version tag.
    if(tag.version_tag) {
        buffer_read_uint_value(buf, buffer_tag::TAG_1, version);

        assert(version == buffer_get_array_version_tag(array));
        if(version != buffer_get_array_version_tag(array)) {
            buf.set_failure();
            return	buf;
        }
    }

    for(unsigned long i = 0; i < size; ++i) {
        buffer_read_array_item(buf, array);
    }

    return buf;
}

//
//	Object:		auto generated like arrays.
//

//
//	Raw:		auto generated.
//
buffer&		operator<<(buffer& buf, const buffer& value);
buffer&		operator>>(buffer& buf, buffer& value);

//
//	None:		buffer_ReadAndIgnore
//

//
//	Version:	auto generated. Using Object Tag field(versioned).
//

//
//	 Enum serial macros.
//
#define		BUFFER_SERIAL_ENUM_(ENUM_TYPE)							\
inline	buffer& operator<<(buffer& buf, const ENUM_TYPE& value)		\
{																	\
	return	buf <<  int(value);										\
}																	\
inline	buffer& operator>>(buffer& buf, ENUM_TYPE& value)			\
{																	\
	int value_;														\
	buf >> value_;													\
	value = ENUM_TYPE(value_);										\
	return buf;														\
}

//
//	safe array.
//
template<typename SAFE_ARRAY_TYPE, int SAFE_ARRAY_SIZE>
inline buffer& operator<<(buffer& buf, const safe_array<SAFE_ARRAY_TYPE, SAFE_ARRAY_SIZE>& array) {
    unsigned long	version		= buffer_get_array_version_tag(array);
    buffer_tag		tag	= {
        buffer_tag::TYPE_ARRAY,
        buffer_size_tag(SAFE_ARRAY_SIZE),
        version >= 2
    };

    //	begin tag.
    if(		!buffer_write_tag(buf, tag)
            ||	!buffer_write_uint_value(buf, tag.size_tag, SAFE_ARRAY_SIZE)
      ) {
        buf.set_failure();
        return	buf;
    }

    //	version tag.
    if(tag.version_tag) {
        buffer_write_uint_value(buf, buffer_tag::TAG_1, version);
    }

    //	content.
    for(size_t i = 0; i < SAFE_ARRAY_SIZE; ++i) {
        buf	<< array[i];
    }

    return buf;
}

template<typename SAFE_ARRAY_TYPE, int SAFE_ARRAY_SIZE>
inline buffer& operator>>(buffer& buf, safe_array<SAFE_ARRAY_TYPE, SAFE_ARRAY_SIZE>& array) {
    unsigned long	size;
    unsigned long	version;
    buffer_tag		tag;
    if(		!buffer_read_tag(buf, tag)
            ||	tag.data_type != buffer_tag::TYPE_ARRAY
            ||	!buffer_read_uint_value(buf, tag.size_tag, size)
            ||	size != SAFE_ARRAY_SIZE
      ) {
        buf.set_failure();
        return	buf;
    }

    //	version tag.
    if(tag.version_tag) {
        buffer_read_uint_value(buf, buffer_tag::TAG_1, version);

        assert(version == buffer_get_array_version_tag(array));
        if(version != buffer_get_array_version_tag(array)) {
            buf.set_failure();
            return	buf;
        }
    }

    for(size_t i = 0; i < SAFE_ARRAY_SIZE; ++i) {
        buf	>> array[i];
    }

    return buf;
}

//
//	缓存多个buffer，常用于数据包的缓存和发送
//
//	接收时，bCanMergeBuffer设置为false，发送时设置为true
//
class	buffer_queue {
public:
    explicit			buffer_queue(bool bCanMergeBuffer = true, unsigned long nMaxBufferSize	= BUFFER_DEFAULT_SIZE);
    ~buffer_queue();

public:
    template<typename DataType>
    bool				serialize(const DataType& data) {
        buffer*	pBuffer	= create();
        pBuffer->clear();
        (*pBuffer)	<<	data;
        return	push(pBuffer);
    }

    //	清空缓存数据包
    void				clear();
    //	创建新的数据包，由调用者手动销毁
    buffer*				create();
    void				destroy(buffer* buf);
    //	追加新的数据包，由被追加者销毁
    bool				push(buffer* buf);
    //	按顺序获取缓存的数据包（可能被合并），由缓存负责销毁[根据buf是否被使用过确定(pos())]
    bool				get(buffer*& buf);

private:
    struct	Data;
    Data*	m_pData;

private:
    buffer_queue(const buffer_queue& rhs);
    buffer_queue& operator=(const buffer_queue& rhs);
};

#endif
