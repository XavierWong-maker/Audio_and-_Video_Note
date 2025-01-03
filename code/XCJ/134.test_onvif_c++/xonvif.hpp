#ifndef TEST_ONVIF_C_XONVIF_HPP
#define TEST_ONVIF_C_XONVIF_HPP

#include <memory>
#include <list>
#include <sstream>

struct soap;
struct SOAP_ENV__Header;
struct wsdd__ScopesType;
struct wsdd__ProbeType;

class XOnvif;
using XOnvif_sp = std::shared_ptr<XOnvif>;

class XOnvif final {

    explicit XOnvif();
    bool construct(const int &timeout_sec);
public:
    ~XOnvif();
    static XOnvif_sp new_XOnvif(const int &timeout_sec);

    using CamList = std::list<std::string>;
    [[maybe_unused]] int Detect_Cams(CamList & cam_list);
    [[maybe_unused]] bool MediaUrl(const std::string &device,
                                      const std::string &user,
                                      const std::string &passwd,
                                      std::string &url);
    bool Profiles(const std::string &media_url,
                      std::string &main_token,
                      std::string &sub_token,
                      const std::string &user,
                      const std::string &pass);


    bool StreamUri(const std::string& media_url,
                   const std::string &token,
                   std::string &rtsp,
                   const std::string &user,
                   const std::string &passwd);

    bool OnvifRtsp(const std::string &device,
                    std::string &main_rtsp,
                    std::string &sub_rtsp,
                    const std::string &user,
                    const std::string &passwd);

private:
    soap * m_soap_{};
    SOAP_ENV__Header *m_header_{};
    wsdd__ScopesType * m_scopes_{};
    wsdd__ProbeType * m_probe_{};
public:
    XOnvif(const XOnvif&) = delete;
    XOnvif(XOnvif&&) = delete;
    XOnvif&operator=(const XOnvif&) = delete;
    XOnvif&operator=(XOnvif&&) = delete;
};

#endif
