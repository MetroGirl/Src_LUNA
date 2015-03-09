#pragma once
#include <DirectXMath.h>
#include <boost/optional.hpp>

struct camera_command
{
  camera_command()
    : yaw_(0.f), pitch_(0.f), move_x_(0.f), move_y_(0.f), zoom_(0.f)
  {}

  camera_command& yaw(float yaw)
  {
    yaw_ = yaw; return *this;
  }

  camera_command& pitch(float pitch)
  {
    pitch_ = pitch; return *this;
  }

  camera_command& move_x(float x)
  {
    move_x_ = x; return *this;
  }

  camera_command& move_y(float y)
  {
    move_y_ = y; return *this;
  }

  camera_command& zoom(float zoom)
  {
    zoom_ = zoom; return *this;
  }

  float yaw_,
        pitch_,
        move_x_,
        move_y_,
        zoom_;
};

struct camera_reset
{
  camera_reset& eye(float x, float y, float z)
  {
    eye_ = DirectX::XMFLOAT3(x, y, z);
    return *this;
  }

  camera_reset& at(float x, float y, float z)
  {
    at_ = DirectX::XMFLOAT3(x, y, z);
    return *this;
  }

  camera_reset& up(float x, float y, float z)
  {
    up_ = DirectX::XMFLOAT3(x, y, z);
    return *this;
  }

  camera_reset& fov_y(float fov_y)
  {
    fov_y_ = fov_y;
    return *this;
  }

  camera_reset& screen(float width, float height)
  {
    screen_width_  = width;
    screen_height_ = height;
    return *this;
  }

  camera_reset& screen_width(float width)
  {
    screen_width_ = width;
    return *this;
  }

  camera_reset& screen_height(float height)
  {
    screen_height_ = height;
    return *this;
  }

  camera_reset& near_z(float z)
  {
    near_z_ = z;
    return *this;
  }

  camera_reset& far_z(float z)
  {
    far_z_ = z;
    return *this;
  }

  boost::optional<DirectX::XMFLOAT3> eye_,
                          at_,
                          up_;
  boost::optional<float>  fov_y_,
                          screen_width_,
                          screen_height_,
                          near_z_, far_z_;
};

class camera
{
public:
  explicit camera(camera_reset const& reset)
    : zoom_out_threshold_(200.f)
  {
    this->reset(reset);
  }

  void reset(camera_reset const& reset)
  {
    if (reset.eye_)    eye_ = *reset.eye_;
    if (reset.at_)     at_  = *reset.at_;
    if (reset.up_)     up_  = *reset.up_;
    if (reset.fov_y_)  fov_y_ = *reset.fov_y_;
    if (reset.screen_width_)  screen_width_  = *reset.screen_width_;
    if (reset.screen_height_) screen_height_ = *reset.screen_height_;
    if (reset.near_z_) near_z_ = *reset.near_z_;
    if (reset.far_z_)  far_z_  = *reset.far_z_;
  }

  void update(camera_command const& command)
  {
    using namespace DirectX;

    auto at  = XMLoadFloat3(&at_);
    auto up  = XMLoadFloat3(&up_);

    auto at_eye = XMVectorSubtract(at, XMLoadFloat3(&eye_));
    auto axis   = XMVector3Normalize(XMVector3Cross(XMVector3Normalize(at_eye), up));
    
    auto pitch = XMMatrixRotationAxis(axis, command.pitch_);
    auto yaw   = XMMatrixRotationY(command.yaw_);

    // zoom
    {
      auto eye = XMLoadFloat3(&eye_);
      if (command.zoom_ != 0.f) {
        auto l    = (std::min)(XMVectorGetX(XMVector3Length(at_eye)), zoom_out_threshold_);
        auto move = l * command.zoom_;
        auto eye_at = XMVectorSubtract(eye, at);
        auto new_eye = XMVectorAdd(at, XMVectorAdd(eye_at, eye_at*move*0.01f));
        XMStoreFloat3(&eye_, new_eye);
      }
    }

    // rotation
    {
      auto eye = XMLoadFloat3(&eye_);
      auto eye_at = XMVectorSubtract(eye, at);
      auto rotation = XMVector3TransformNormal(eye_at, XMMatrixMultiply(pitch, yaw));
      auto new_eye = XMVectorAdd(at, rotation);
      XMStoreFloat3(&eye_, new_eye);
    }

    // move
    {
      auto eye = XMLoadFloat3(&eye_);
      auto eye_at = XMVectorSubtract(eye, at);
      auto dir = XMVector3Normalize(eye_at);
      auto x_axis = XMVector3Cross(dir, XMVectorSet(0, 1, 0, 0));
      auto rotation = XMMatrixRotationAxis(x_axis, g_XMPi.f[0]/2.f);
      auto y_axis = XMVector3TransformNormal(dir, rotation);
      auto new_eye = eye + (x_axis * command.move_x_) + (y_axis * command.move_y_);
      auto new_at  = at  + (x_axis * command.move_x_) + (y_axis * command.move_y_);
      XMStoreFloat3(&eye_, new_eye);
      XMStoreFloat3(&at_, new_at);
    }
  }

  DirectX::XMFLOAT4X4 calculate_view_matrix() const
  {
    using namespace DirectX;

    auto view = XMMatrixLookAtLH(XMLoadFloat3(&eye_), XMLoadFloat3(&at_), XMLoadFloat3(&up_));
    XMFLOAT4X4 ret;
    XMStoreFloat4x4(&ret, view);
    return ret;
  }

private:
  DirectX::XMFLOAT3 eye_, at_, up_;
  float             fov_y_;
  float             screen_width_;
  float             screen_height_;
  float             zoom_factor_;
  float             zoom_out_threshold_;
  float             near_z_, far_z_;
};
