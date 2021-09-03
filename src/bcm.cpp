//
// Created by Lukas Miklosko on 8/16/21.
//

#include <array>
#include <memory>
#include <cstdio>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "exceptions.hpp"
#include "bcm_host.hpp"

#ifndef MAP_LOCKED
#define MAP_LOCKED 0
#endif

static unsigned get_dt_ranges(const char *filename, unsigned offset)
{
    unsigned address = ~0;
    FILE *fp = fopen(filename, "rb");
    if (fp)
    {
        unsigned char buf[4];
        fseek(fp, offset, SEEK_SET);
        if (fread(buf, 1, sizeof buf, fp) == sizeof buf)
            address = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3] << 0;
        fclose(fp);
    }
    return address;
}

unsigned bcm_getPeripheralAddress()
{
    unsigned address = get_dt_ranges("/proc/device-tree/soc/ranges", 4);
    if (address == 0)
        address = get_dt_ranges("/proc/device-tree/soc/ranges", 8);
    return address == ~0U ? 0x20000000 : address;
}

unsigned bcm_getPeripheralSize()
{
    unsigned address = get_dt_ranges("/proc/device-tree/soc/ranges", 4);
    address = get_dt_ranges("/proc/device-tree/soc/ranges", (address == 0) ? 12 : 8);
    return address == ~0U ? 0x01000000 : address;
}

template<typename Tp, std::size_t N>
class Storage
{
    static constexpr auto deleter = [](Tp* ptr) { munmap((void*)(ptr), sizeof(Tp)); };

    template<size_t ... indices>
    explicit Storage(std::index_sequence<indices...> const&)
        : ptr{((void)indices, ptr_type(nullptr, deleter))...}
    {
    }
public:
    using ptr_type = std::unique_ptr<Tp, decltype(deleter)>;
    std::array<ptr_type, N> ptr;

    Storage() : Storage(std::make_index_sequence<N>{}) {}
};
#include <iostream>
template<typename Tp, size_t nControllers = 1, size_t offset = 0>
static Tp* getPeripheralPtr(unsigned address, int idx = 0)
{
    static Storage<Tp, nControllers> storage;

    if (!storage.ptr[idx])
    {
        int fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC);
        if (fd < 0)
        {
            throw LLD::access_exception{};
        }

        auto virtaddr = mmap(nullptr, sizeof(Tp), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, bcm_getPeripheralAddress() | (address + offset * idx));
        if (virtaddr == MAP_FAILED)
        {
            throw LLD::memory_access_exception{};
        }

        close(fd);
        storage.ptr[idx].reset(reinterpret_cast<Tp*>(virtaddr));
    }

    return storage.ptr[idx].get();
}

[[maybe_unused]]
volatile dma_base_t* bcm_dmaPerip()
{
    return getPeripheralPtr<volatile dma_base_t>(0x00007000);
}
[[maybe_unused]]
volatile power_management_t* bcm_pmPerip()
{
    return getPeripheralPtr<volatile power_management_t>(0x00100000);
}
[[maybe_unused]]
volatile clock_management_t* bcm_clkPerip()
{
    return getPeripheralPtr<volatile clock_management_t>(0x00101000);
}
[[maybe_unused]]
volatile gpio_base_t* bcm_gpioPerip()
{
    return getPeripheralPtr<volatile gpio_base_t>(0x00200000);
}
[[maybe_unused]]
volatile pcm_base_t* bcm_pcmPerip()
{
    return getPeripheralPtr<volatile pcm_base_t>(0x00203000);
}
[[maybe_unused]]
volatile pwm_base_t *bcm_pwmPerip(std::size_t idx)
{
    return getPeripheralPtr<volatile pwm_base_t, 2, 0x800>(0x0020C000, idx);
}
[[maybe_unused]]
volatile a2w_base_t* bcm_a2wPerip()
{
    return getPeripheralPtr<volatile a2w_base_t>(0x00102000);
}