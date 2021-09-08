#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>
#include <string>
#include <cstring>

namespace LLD
{

/**
 *  Base LLD exception class
 *
 *  All further exception are derived from this class.
 */
struct lowleveldevices_exception : public std::exception
{
    [[nodiscard]]
    /* virtual */ const char* what() const noexcept override
    {
        return "Generic low level devices exception occurred.";
    }
};


/**
 *  No access exception
 *
 */
struct access_exception : public lowleveldevices_exception
{
    [[nodiscard]]
    /* virtual */ const char* what() const noexcept override
    {
        return "Unable to access resource (file, device or stream).";
    }
};

/**
 * No memory access exception.
 *
 * @detail used when the underlying operating system denied
 * access to the peripheral address range.
 */
struct memory_access_exception : public access_exception
{
    [[nodiscard]]
    /* virtual */ const char* what() const noexcept override
    {
        return "Direct access to the peripheral registers denied.";
    }
};

/**
 *  Access violation exception
 *
 *  Requested pin/device is already open
 */
struct access_violation_exception : public access_exception
{
    [[nodiscard]]
    /* virtual */ const char* what() const noexcept override
    {
        return "Requested pin has already been opened.";
    }
};

/**
 * Not found exception
 *
 * This exception is thrown when appropriate device, file or content could not be found.
 */
struct not_found_exception : public lowleveldevices_exception
{
    [[nodiscard]]
    /* virtual */ const char* what() const noexcept override
    {
        return "The requested device, file or content could not be found. "
            "Please make sure you are using supported device / operating system.";
    }
};

/**
 *  No device controller
 *
 *  The system does not have requested device controller installed.
 */
struct no_controller_exception : public lowleveldevices_exception
{
    [[nodiscard]]
    /* virtual */ const char* what() const noexcept override
    {
        return "The requested device controller could not be found on the system.";
    }
};

/**
 *  Not supported exception
 *
 */
struct not_supported_exception : public lowleveldevices_exception
{
    [[nodiscard]]
    /* virtual */ const char* what() const noexcept override
    {
        return "Requested operation or mode is not supported by the controller";
    }
};

/**
 *  Invalid Argument passed into function
 *
 *  exp and got members hold expected and received argument
 */
struct invalid_argument_exception : public lowleveldevices_exception
{
    invalid_argument_exception(const std::string& fn, const std::string& exp, const std::string& got) :
        msg("Invalid argument (" + got + ") was passed into " + fn + " function. \n\tExpected: " + exp)
    {
    }

    [[nodiscard]]
    /* virtual */ const char* what() const noexcept override
    {
        return msg.c_str();
    }

private:
    std::string msg;
};


/**
 *  IOCTL Failure exception
 *
 *  IOCTL Call @member OP failed with error @member err for @member fd descriptor
 */
struct ioctl_exception : public lowleveldevices_exception
{
    ioctl_exception(int fd, const char* op) : err(errno), fd(fd), op(op) 
    {
        sprintf(msg, "IOCTL call %s failed for the device %x with errno %d: %s", op, fd, err, strerror(err));
    }

    [[nodiscard]]
    /* virtual */ const char* what() const noexcept override
    {
        return msg;
    }

    int err;
    int fd;
    const char* op;

private:
    char msg[128]{};
};

}

#endif // EXCEPTIONS_HPP