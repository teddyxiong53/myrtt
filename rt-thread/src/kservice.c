#include <rthw.h>
#include <rtthread.h>

#define ZEROPAD (1<<0)
#define SIGN	(1<<1)
#define PLUS	(1<<2)
#define SPACE 	(1<<3)
#define LEFT	(1<<4)
#define SPECIAL (1<<5)
#define LARGE	(1<<6) //use ABCEDF instead of abcdef

static rt_device_t _console_device = RT_NULL;


rt_inline int skip_atoi(const char **s)
{
	register int i=0;
	while(isdigit(**s)) {
		i = i*10 + *((*s)++) - '0';
	}
	return i;
}

rt_size_t rt_strlen(const char *s)
{
	const char *sc;
	for(sc=s; *sc!='\0'; ++sc) {

	}
	return sc -s;
	
}
#define isdigit(c)  ((unsigned)((c) - '0') <10)

rt_inline rt_int32_t divide(rt_int32_t *n, rt_int32_t base)
{
	rt_int32_t res;
	if(base == 10) {
		res = ((rt_uint32_t)*n)%10U;
		*n = ((rt_uint32_t)*n)/10U;
	} else {
		res = ((rt_uint32_t)*n)%16U;
		*n = ((rt_uint32_t)*n)/16U;
	}
	return res;
}

static char *print_number(
	char *buf,
	char *end,
	long num,
	int base,
	int s,
	int precision,
	int type
)
{
	char c, sign;
	char tmp[16];
	const char *digits;
	static const char small_digits[] = "01234567890abcdef";
	static const char large_digits[] = "01234567890ABCDEF";
	register int i,size;

	size = s;
	digits = (type&LARGE) ? large_digits : small_digits;
	if(type & LEFT) {
		type &= ~ZEROPAD;
	}
	c = (type&ZEROPAD)?'0': ' ';
	sign = 0;
	if(type&SIGN) {
		if(num <0) {
			sign = '-';
			num = -num;
		} else if(type&PLUS) {
			sign = '+';
		} else if(type & SPACE) {
			sign = ' ';
		}
	}

	i = 0;
	if(num == 0) {
		tmp[i++] = '0';
	} else {
		while(num != 0) {
			tmp[i++] = digits[divide(&num,base)];
		}
	}
	if(i>precision) {
		precision = i;
	}
	size -= precision;

	if(!(type&(ZEROPAD|LEFT))) {
		if((sign)&&(size>0)) {
			size --;
		}
		while(size-- > 0) {
			if(buf <= end) {
				*buf = ' ';
			}
			++buf;
		}
	}
	if(sign) {
		if(buf <= end) {
			*buf = sign;
			--size;
		}
		++buf;
	}

	if(!(type & LEFT)) {
		while(size-- > 0) {
			if(buf <= end) {
				*buf = c;
			}
			++buf;
		}
	}

	while(i<precision--) {
		if(buf <= end) {
			*buf = '0';
		}
		++buf;
	}

	while(i-- > 0) {
		if(buf <= end) {
			*buf = tmp[i];
		}
		++buf;
	}
	while(size-- > 0) {
		if(buf <= end) {
			*buf = ' ';
		}
		++buf;
	}
	return buf;
}

char *rt_strncpy(char *dst, char *src, rt_ubase_t n)
{
	if(n!=0) {
		char *d = dst;
		char *s = src;
		do {
			if((*d++ = *s++) == 0) {
				while(--n != 0) {
					*d++ = 0;
				}
				break;
			}
		} while(--n != 0);
	}
	return dst;
}

rt_int32_t rt_strncmp(const char *cs, const char *ct, rt_ubase_t count)
{
	char res = 0;
	while(count) {
		if((res = *cs - *ct++)!=0 || !*cs++) {
			break;
		}
		count --;
	}
	return res;
}

rt_device_t rt_console_set_device(const char *name)
{
	rt_device_t old, new;
	old = _console_device;
	new = rt_device_find(name);
	if(new != RT_NULL) {
		if(_console_device != RT_NULL) {
			rt_device_close(_console_device);
		}
		rt_device_open(new, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
		_console_device = new;
	}
	return old;
}


rt_int32_t rt_vsnprintf(char *buf, rt_size_t size, const char *fmt, va_list args)
{
	rt_uint32_t num;
	char *str;
	str = buf;
	rt_uint8_t qualifier;
	rt_uint8_t base;
	int precision;
	char *end;
	const char *s;
	char c;
	int i, len;
	end = buf + size - 1;
	if(end < buf) {
		end = ((char *)(-1));
		size = end - buf;
	}
	rt_uint8_t flags;
	rt_int32_t field_width ;
	for(; *fmt; ++fmt) {
		if(*fmt != '%') {
			if(str <= end) {
				*str = *fmt;
			}
			++str;
			continue;
		}
		flags = 0;
		while(1) {
			++fmt;
			if(*fmt == '-') {
				flags |= LEFT;
			} else if(*fmt == '+') {
				flags |= PLUS;
			} else if(*fmt == ' ') {
				flags |= SPACE;
			} else if(*fmt == '#' ) {
				flags |= SPECIAL;
			} else if(*fmt == '0') {
				flags |= ZEROPAD;
			} else {
				break;
			}
			
		}
		field_width = -1;
		if(isdigit(*fmt)) {
			field_width = skip_atoi(&fmt);
		} else if(*fmt == '*') {
			++fmt;
			field_width = va_arg(args, int);
			if(field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		precision = -1;
		if(*fmt == '.') {
			++fmt;
			if(isdigit(*fmt)) {
				precision = skip_atoi(&fmt);
			} else if(*fmt == '*') {
				++fmt;
				precision = va_arg(args, int);
			}
			if(precision < 0) {
				precision = 0;
			}
		}
		qualifier = 0;
		if(*fmt == 'h' || *fmt == 'l') {
			qualifier = *fmt;
			++fmt;
		
		}
		base = 10;
		switch(*fmt) {
		case 'c':
			if(!(flags&LEFT)) {
				while(--field_width > 0) {
					if(str <= end) {
						*str = ' ';
					}
					++str;
				}
			}
			c = (rt_uint8_t)va_arg(args, int);
			if(str <= end) {
				*str = c;
			}
			++str;
			while(--field_width > 0) {
				if(str <= end) {
					*str = ' ';
					
				}
				++str;
			}
			continue;
		case 's':
			s = va_arg(args, char *);
			if(!s) {
				s = "NULL";
			}
			len = rt_strlen(s);
			if(precision > 0 && len > precision) {
				len = precision;
			}
			if(!(flags & LEFT)) {
				while(len < field_width--) {
					if(str <= end) {
						*str = ' ';
					}
					++str;
				}
			}
			for(i=0; i<len; i++) {
				if(str <= end) {
					*str = *s;
				}
				++str;
				++s;
			}
			while(len < field_width --) {
				if(str <= end) {
					*str = ' ';
				}
				++str;
			}
			continue;
		case 'p':
			if(field_width == -1) {
				field_width = sizeof(void *)<<1;
				flags |= ZEROPAD;
			}
			str = print_number(str, end, (long)va_arg(args, void*), 16, field_width, precision, flags);
			continue;
		case '%':
			if(str <= end) {
				*str = '%';
				
			}
			++ str ;
			continue;
		case 'o':
			base = 8;
			break;
		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;
		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;
		default:
			if(str <= end) {
				*str = '%';
			}
			++str;
			if(*fmt) {
				if(str<=end) {
					*str = *fmt;
					++str;
				} 
				
			}else {
				--fmt;
			}
			continue;
		}

		if(qualifier == 'l') {
			num = va_arg(args, rt_uint32_t);
			if(flags&SIGN) {
				num = (rt_int32_t)num;
			} 
		}else if(qualifier == 'h') {
			num = (rt_uint16_t)va_arg(args, rt_int32_t);
			if(flags & SIGN) {
				num = (rt_int16_t)num;
			}
		} else {
			num = va_arg(args, rt_uint32_t);
			if(flags & SIGN) {
				num = (rt_uint32_t)num;
			}
		}
		str = print_number(str, end, num, base, field_width, precision, flags);
		
	}

	if(str <= end) {
		*str = '\0';
	} else {
		*end = '\0';
	}
	return str - buf;
}

void rt_kprintf(const char *fmt, ...)
{
	va_list args;
	rt_size_t length;
	static char rt_log_buf[RT_CONSOLEBUF_SIZE];
	va_start(args, fmt);
	//the return val of vsnprintf is the 
	length = rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf)-1, fmt, args);
	if(length > RT_CONSOLEBUF_SIZE-1) {
		length = RT_CONSOLEBUF_SIZE - 1;
	}
	if(_console_device == RT_NULL) {
		//rt_hw_console_output(rt_log_buf);
	} else {
		rt_uint16_t old_flag = _console_device->open_flag;
		_console_device->open_flag |= RT_DEVICE_FLAG_STREAM;
		rt_device_write(_console_device, 0, rt_log_buf);
		_console_device->open_flag = old_flag;
	}
	va_end(args);
}

void rt_show_version()
{
	rt_kprintf("rt-thread, version:%d.%d.%d build %s", RT_VERION, RT_SUBVERSION,
		RT_REVISION, __DATE__);
	
}

