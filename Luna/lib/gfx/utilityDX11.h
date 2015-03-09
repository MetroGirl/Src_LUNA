#pragma once
#include <d3d11.h>
#include <vector>

namespace luna {

ID3D11ShaderResourceView* const   null_srvs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
ID3D11UnorderedAccessView* const  null_uavs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }; 

template <class ShaderT>
inline void set_srv_impl(ID3D11DeviceContext& dc, ::UINT start_slot, ::UINT num_srvs, ID3D11ShaderResourceView* const* srvs);
template <>
inline void set_srv_impl<ID3D11VertexShader>(ID3D11DeviceContext& dc, ::UINT start_slot, ::UINT num_srvs, ID3D11ShaderResourceView* const* srvs)
{
  dc.VSSetShaderResources(start_slot, num_srvs, srvs);
}
template <>
inline void set_srv_impl<ID3D11HullShader>(ID3D11DeviceContext& dc, ::UINT start_slot, ::UINT num_srvs, ID3D11ShaderResourceView* const* srvs)
{
  dc.HSSetShaderResources(start_slot, num_srvs, srvs);
}
template <>
inline void set_srv_impl<ID3D11DomainShader>(ID3D11DeviceContext& dc, ::UINT start_slot, ::UINT num_srvs, ID3D11ShaderResourceView* const* srvs)
{
  dc.DSSetShaderResources(start_slot, num_srvs, srvs);
}
template <>
inline void set_srv_impl<ID3D11GeometryShader>(ID3D11DeviceContext& dc, ::UINT start_slot, ::UINT num_srvs, ID3D11ShaderResourceView* const* srvs)
{
  dc.GSSetShaderResources(start_slot, num_srvs, srvs);
}
template <>
inline void set_srv_impl<ID3D11PixelShader>(ID3D11DeviceContext& dc, ::UINT start_slot, ::UINT num_srvs, ID3D11ShaderResourceView* const* srvs)
{
  dc.PSSetShaderResources(start_slot, num_srvs, srvs);
}
template <>
inline void set_srv_impl<ID3D11ComputeShader>(ID3D11DeviceContext& dc, ::UINT start_slot, ::UINT num_srvs, ID3D11ShaderResourceView* const* srvs)
{
  dc.CSSetShaderResources(start_slot, num_srvs, srvs);
}

struct dispatch_xy
{
  dispatch_xy(::UINT x, ::UINT y)
    : x_(x), y_(y)
  {}

  ::UINT x_, y_;
};

template <class T>
struct resource_array
{
  template <class Range>
  resource_array(Range& range)
    : _(std::begin(range), std::end(range))
  {}

  resource_array(std::initializer_list<T> const& il)
    : _(std::begin(il), std::end(il))
  {}

  template <std::size_t Size>
  resource_array(T (*range)[Size])
    : _(range, &range[Size])
  {}

  std::vector<T>  _;
};

typedef resource_array<ID3D11ShaderResourceView*>  srv_array;
typedef resource_array<ID3D11UnorderedAccessView*> uav_array;
typedef resource_array<::UINT>  uint_array;

template <class ShaderT>
struct scoped_srv
{
  scoped_srv(ID3D11DeviceContext& dc, ::UINT start_slot, srv_array const& srvs)
    : dc_(dc)
    , start_slot_(start_slot)
    , size_(srvs._.size())
  {
    set_srv_impl<ShaderT>(dc, start_slot, srvs._.size(), srvs._.data());
  }

  ~scoped_srv()
  {
    set_srv_impl<ShaderT>(dc_, start_slot_, size_, null_srvs);
  }

  ID3D11DeviceContext&  dc_;
  ::UINT const          start_slot_;
  std::size_t const     size_;
};

typedef scoped_srv<ID3D11VertexShader>  scoped_srv_vs;
typedef scoped_srv<ID3D11HullShader>    scoped_srv_hs;
typedef scoped_srv<ID3D11DomainShader>  scoped_srv_ds;
typedef scoped_srv<ID3D11GeometryShader>  scoped_srv_gs;
typedef scoped_srv<ID3D11PixelShader>   scoped_srv_ps;
typedef scoped_srv<ID3D11ComputeShader> scoped_srv_cs;

struct scoped_uav_cs
{
  scoped_uav_cs(ID3D11DeviceContext& dc, ::UINT start_slot, uav_array const& uavs)
    : dc_(dc)
    , start_slot_(start_slot)
    , size_(uavs._.size())
  {
    dc_.CSSetUnorderedAccessViews(start_slot, uavs._.size(), uavs._.data(), nullptr);
  }

  scoped_uav_cs(ID3D11DeviceContext& dc, ::UINT start_slot, uav_array const& uavs, uint_array const& initial_counters)
    : dc_(dc)
    , start_slot_(start_slot)
    , size_(uavs._.size())
  {
    dc_.CSSetUnorderedAccessViews(start_slot, uavs._.size(), uavs._.data(), initial_counters._.data());
  }

  ~scoped_uav_cs()
  {
    dc_.CSSetUnorderedAccessViews(start_slot_, size_, null_uavs, nullptr);
  }

  ID3D11DeviceContext&  dc_;
  ::UINT const          start_slot_;
  std::size_t const     size_;
};

}
