// SPDX-License-Identifier: MIT
// Copyright © 2024 Julien Bernard <julien.bernard@univ-fcomte.fr>
#ifndef CAIROPP_H
#define CAIROPP_H

#include <cassert>

#include <array>
#include <filesystem>
#include <iterator>
#include <memory>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <cairo.h>
#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif

namespace cairo {

  namespace details {

    struct IncreaseReferenceType {};
    constexpr IncreaseReferenceType IncreaseReference = {};

    template<typename T, void (*Destroy)(T*)>
    class BasicHandle {
    public:
      BasicHandle() = default;

      BasicHandle(T* object)
      : m_object(object)
      {
      }

      BasicHandle(const BasicHandle&) = default;

      BasicHandle(BasicHandle&& other) noexcept
      : m_object(std::exchange(other.m_object, nullptr))
      {
      }

      ~BasicHandle() {
        destroy();
      }

      BasicHandle& operator=(const BasicHandle&) = default;

      BasicHandle& operator=(BasicHandle&& other) noexcept
      {
        if (&other == this) {
          return *this;
        }

        destroy();
        m_object = std::exchange(other.m_object, nullptr);
        return *this;
      }

      T* get() noexcept
      {
        return m_object;
      }

      const T* get() const noexcept
      {
        return m_object;
      }

      void set(T* object)
      {
        m_object = object;
      }

      operator T*() noexcept
      {
        return m_object;
      }

      operator const T*() const noexcept
      {
        return m_object;
      }

      void destroy()
      {
        if (m_object != nullptr) {
          Destroy(m_object);
        }
      }

    private:
      T* m_object = nullptr;
    };



    template<typename T, T* (*Reference)(T*), void (*Destroy)(T*)>
    class Handle : public BasicHandle<T, Destroy> {
    public:
      using BasicHandle<T, Destroy>::BasicHandle;
      using BasicHandle<T, Destroy>::destroy;
      using BasicHandle<T, Destroy>::get;
      using BasicHandle<T, Destroy>::set;

      Handle(T* object, [[maybe_unused]] IncreaseReferenceType incr)
      : BasicHandle<T, Destroy>(object)
      {
        reference();
      }

      Handle(const Handle& other)
      : BasicHandle<T, Destroy>(other.get())
      {
        reference();
      }

      Handle(Handle&&) noexcept = default;

      ~Handle() = default;

      Handle& operator=(const Handle& other)
      {
        if (&other == this) {
          return *this;
        }

        destroy();
        set(other.get());
        reference();
        return *this;
      }

      Handle& operator=(Handle&&) noexcept = default;

      void reference()
      {
        if (get() != nullptr) {
          Reference(get());
        }
      }
    };

    template<typename T, T* (*Copy)(const T*), void (*Destroy)(T*)>
    class CopyableHandle : public BasicHandle<T, Destroy> {
    public:
      using BasicHandle<T, Destroy>::BasicHandle;
      using BasicHandle<T, Destroy>::destroy;
      using BasicHandle<T, Destroy>::get;
      using BasicHandle<T, Destroy>::set;

      CopyableHandle(const CopyableHandle& other)
      : BasicHandle<T, Destroy>(Copy(other.get()))
      {
      }

      CopyableHandle(CopyableHandle&&) noexcept = default;

      ~CopyableHandle() = default;

      CopyableHandle& operator=(const CopyableHandle& other)
      {
        if (&other == this) {
          return *this;
        }

        destroy();
        set(Copy(other.get()));
        return *this;
      }

      CopyableHandle& operator=(CopyableHandle&&) noexcept = default;
    };

    template<typename T, void (*Destroy)(T*)>
    class NonCopyableHandle : public BasicHandle<T, Destroy> {
    public:
      using BasicHandle<T, Destroy>::BasicHandle;

      NonCopyableHandle(const NonCopyableHandle&) = delete;
      NonCopyableHandle(NonCopyableHandle&&) noexcept = default;
      ~NonCopyableHandle() = default;
      NonCopyableHandle& operator=(const NonCopyableHandle&) = delete;
      NonCopyableHandle& operator=(NonCopyableHandle&&) noexcept = default;
    };

  }

  // utilities

  template<typename T>
  struct Vec2 {
    T x;
    T y;
  };

  using Vec2F = Vec2<double>;
  using Vec2I = Vec2<int>;

  template<typename T>
  struct Rect {
    T x;
    T y;
    T w;
    T h;
  };

  using RectF = Rect<double>;
  using RectI = Rect<int>;

  struct Color {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    double a = 1.0;
  };

  // binding

  enum class Status : std::underlying_type_t<cairo_status_t> { // NOLINT(performance-enum-size)
    Success = CAIRO_STATUS_SUCCESS,
    NoMemory = CAIRO_STATUS_NO_MEMORY,
    InvalidRestore = CAIRO_STATUS_INVALID_RESTORE,
    InvalidPopGroup = CAIRO_STATUS_INVALID_POP_GROUP,
    NoCurrentPoint = CAIRO_STATUS_NO_CURRENT_POINT,
    InvalidMatrix = CAIRO_STATUS_INVALID_MATRIX,
    InvalidStatus = CAIRO_STATUS_INVALID_STATUS,
    NullPointer = CAIRO_STATUS_NULL_POINTER,
    InvalidString = CAIRO_STATUS_INVALID_STRING,
    InvalidPathData = CAIRO_STATUS_INVALID_PATH_DATA,
    ReadError = CAIRO_STATUS_READ_ERROR,
    WriteError = CAIRO_STATUS_WRITE_ERROR,
    SurfaceFinished = CAIRO_STATUS_SURFACE_FINISHED,
    SurfaceTypeMismatch = CAIRO_STATUS_SURFACE_TYPE_MISMATCH,
    PatternTypeMismatch = CAIRO_STATUS_PATTERN_TYPE_MISMATCH,
    InvalidContent = CAIRO_STATUS_INVALID_CONTENT,
    InvalidFormat = CAIRO_STATUS_INVALID_FORMAT,
    InvalidVisual = CAIRO_STATUS_INVALID_VISUAL,
    FileNotFound = CAIRO_STATUS_FILE_NOT_FOUND,
    InvalidDash = CAIRO_STATUS_INVALID_DASH,
    InvalidDscComment = CAIRO_STATUS_INVALID_DSC_COMMENT,
    InvalidIndex = CAIRO_STATUS_INVALID_INDEX,
    ClipNotRepresentable = CAIRO_STATUS_CLIP_NOT_REPRESENTABLE,
    TempFileError = CAIRO_STATUS_TEMP_FILE_ERROR,
    InvalidStride = CAIRO_STATUS_INVALID_STRIDE,
    FontTypeMismatch = CAIRO_STATUS_FONT_TYPE_MISMATCH,
    UserFontImmutable = CAIRO_STATUS_USER_FONT_IMMUTABLE,
    UserFontError = CAIRO_STATUS_USER_FONT_ERROR,
    NegativeCount = CAIRO_STATUS_NEGATIVE_COUNT,
    InvalidClusters = CAIRO_STATUS_INVALID_CLUSTERS,
    InvalidSlant = CAIRO_STATUS_INVALID_SLANT,
    InvalidWeight = CAIRO_STATUS_INVALID_WEIGHT,
    InvalidSize = CAIRO_STATUS_INVALID_SIZE,
    UserFontNotImplemented = CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED,
    DeviceTypeMismatch = CAIRO_STATUS_DEVICE_TYPE_MISMATCH,
    DeviceError = CAIRO_STATUS_DEVICE_ERROR,
    InvalidMeshConstruction = CAIRO_STATUS_INVALID_MESH_CONSTRUCTION,
    DeviceFinished = CAIRO_STATUS_DEVICE_FINISHED,
    Jbig2GlobalMissing = CAIRO_STATUS_JBIG2_GLOBAL_MISSING,
    PngError = CAIRO_STATUS_PNG_ERROR,
    FreetypeError = CAIRO_STATUS_FREETYPE_ERROR,
    Win32GdiError = CAIRO_STATUS_WIN32_GDI_ERROR,
    TagError = CAIRO_STATUS_TAG_ERROR,
  };

  inline std::string_view to_string(Status s) { return cairo_status_to_string(static_cast<cairo_status_t>(s)); }

  enum class Content : std::underlying_type_t<cairo_content_t> { // NOLINT(performance-enum-size)
    Color = CAIRO_CONTENT_COLOR,
    Alpha = CAIRO_CONTENT_ALPHA,
    ColorAlpha = CAIRO_CONTENT_COLOR_ALPHA,
  };

  enum class Format : std::underlying_type_t<cairo_format_t> { // NOLINT(performance-enum-size)
    Invalid = CAIRO_FORMAT_INVALID,
    Argb32 = CAIRO_FORMAT_ARGB32,
    Rgb24 = CAIRO_FORMAT_RGB24,
    A8 = CAIRO_FORMAT_A8,
    A1 = CAIRO_FORMAT_A1,
    Rgb16_565 = CAIRO_FORMAT_RGB16_565,
    Rgb30 = CAIRO_FORMAT_RGB30,
  };

  enum class Operator : std::underlying_type_t<cairo_operator_t> { // NOLINT(performance-enum-size)
    Clear = CAIRO_OPERATOR_CLEAR,

    Source = CAIRO_OPERATOR_SOURCE,
    Over = CAIRO_OPERATOR_OVER,
    In = CAIRO_OPERATOR_IN,
    Out = CAIRO_OPERATOR_OUT,
    Atop = CAIRO_OPERATOR_ATOP,

    Dest = CAIRO_OPERATOR_DEST,
    DestOver = CAIRO_OPERATOR_DEST_OVER,
    DestIn = CAIRO_OPERATOR_DEST_IN,
    DestOut = CAIRO_OPERATOR_DEST_OUT,
    DestAtop = CAIRO_OPERATOR_DEST_ATOP,

    Xor = CAIRO_OPERATOR_XOR,
    Add = CAIRO_OPERATOR_ADD,
    Saturate = CAIRO_OPERATOR_SATURATE,

    Multiply = CAIRO_OPERATOR_MULTIPLY,
    Screen = CAIRO_OPERATOR_SCREEN,
    Overlay = CAIRO_OPERATOR_OVERLAY,
    Darken = CAIRO_OPERATOR_DARKEN,
    Lighten = CAIRO_OPERATOR_LIGHTEN,
    ColorDodge = CAIRO_OPERATOR_COLOR_DODGE,
    ColorBurn = CAIRO_OPERATOR_COLOR_BURN,
    HardLight = CAIRO_OPERATOR_HARD_LIGHT,
    SoftLight = CAIRO_OPERATOR_SOFT_LIGHT,
    Difference = CAIRO_OPERATOR_DIFFERENCE,
    Exclusion = CAIRO_OPERATOR_EXCLUSION,
    HslHue = CAIRO_OPERATOR_HSL_HUE,
    HslSaturation = CAIRO_OPERATOR_HSL_SATURATION,
    HslColor = CAIRO_OPERATOR_HSL_COLOR,
    HslLuminosity = CAIRO_OPERATOR_HSL_LUMINOSITY,
  };

  enum class Antialias : std::underlying_type_t<cairo_antialias_t> { // NOLINT(performance-enum-size)
    Default = CAIRO_ANTIALIAS_DEFAULT,

    None = CAIRO_ANTIALIAS_NONE,
    Gray = CAIRO_ANTIALIAS_GRAY,
    Subpixel = CAIRO_ANTIALIAS_SUBPIXEL,

    Fast = CAIRO_ANTIALIAS_FAST,
    Good = CAIRO_ANTIALIAS_GOOD,
    Best = CAIRO_ANTIALIAS_BEST,
  };

  enum class FillRule : std::underlying_type_t<cairo_fill_rule_t> { // NOLINT(performance-enum-size)
    Winding = CAIRO_FILL_RULE_WINDING,
    EvenOdd = CAIRO_FILL_RULE_EVEN_ODD,
  };

  enum class LineCap : std::underlying_type_t<cairo_line_cap_t> { // NOLINT(performance-enum-size)
    Butt = CAIRO_LINE_CAP_BUTT,
    Round = CAIRO_LINE_CAP_ROUND,
    Square = CAIRO_LINE_CAP_SQUARE,
  };

  enum class LineJoin : std::underlying_type_t<cairo_line_join_t> { // NOLINT(performance-enum-size)
    Miter = CAIRO_LINE_JOIN_MITER,
    Round = CAIRO_LINE_JOIN_ROUND,
    Bevel = CAIRO_LINE_JOIN_BEVEL,
  };

  /*
   * matrix
   */

  struct Matrix : private cairo_matrix_t {

    static Matrix create(double  xx, double  yx, double  xy, double  yy, double  x0, double  y0) { Matrix m; cairo_matrix_init(m, xx, yx, xy, yy, x0, y0); return m; }
    static Matrix create_identity() { Matrix m; cairo_matrix_init_identity(m); return m; }
    static Matrix create_translate(double tx, double ty) { Matrix m; cairo_matrix_init_translate(m, tx, ty); return m; }
    static Matrix create_scale(double sx, double sy) { Matrix m; cairo_matrix_init_scale(m, sx, sy); return m; }
    static Matrix create_rotate(double radians) { Matrix m; cairo_matrix_init_rotate(m, radians); return m; }

    void translate(double tx, double ty) { cairo_matrix_translate(this, tx, ty); }
    void scale(double sx, double sy) { cairo_matrix_scale(this, sx, sy); }
    void rotate(double radians) { cairo_matrix_rotate(this, radians); }

    Status invert() { return static_cast<Status>(cairo_matrix_invert(this)); }

    Vec2F transform_distance(Vec2F d) const { cairo_matrix_transform_distance(this, &d.x, &d.y); return d; }
    Vec2F transform_point(Vec2F p) const { cairo_matrix_transform_point(this, &p.x, &p.y); return p; }

    Matrix operator*(const Matrix& other) const { Matrix res; cairo_matrix_multiply(res, this, other); return res; }

    operator cairo_matrix_t*()
    {
      return this;
    }

    operator const cairo_matrix_t*() const
    {
      return this;
    }
  };

  /*
   * font
   */

  using glyph = cairo_glyph_t;
  using text_cluster = cairo_text_cluster_t;

  enum class TextClusterFlags : std::underlying_type_t<cairo_text_cluster_flags_t> { // NOLINT(performance-enum-size)
    None = 0,
    Backward = CAIRO_TEXT_CLUSTER_FLAG_BACKWARD,
  };

  // TODO: flags operators

  using TextExtents = cairo_text_extents_t;
  using FontExtents = cairo_font_extents_t;

  enum class FontSlant : std::underlying_type_t<cairo_font_slant_t> { // NOLINT(performance-enum-size)
    Normal = CAIRO_FONT_SLANT_NORMAL,
    Italic = CAIRO_FONT_SLANT_ITALIC,
    Oblique = CAIRO_FONT_SLANT_OBLIQUE,
  };

  enum class FontWeight : std::underlying_type_t<cairo_font_weight_t> { // NOLINT(performance-enum-size)
    Normal = CAIRO_FONT_WEIGHT_NORMAL,
    Bold = CAIRO_FONT_WEIGHT_BOLD,
  };

  enum class SubpixelOrder : std::underlying_type_t<cairo_subpixel_order_t> { // NOLINT(performance-enum-size)
    Default = CAIRO_SUBPIXEL_ORDER_DEFAULT,
    Rgb = CAIRO_SUBPIXEL_ORDER_RGB,
    Bgr = CAIRO_SUBPIXEL_ORDER_BGR,
    Vrgb = CAIRO_SUBPIXEL_ORDER_VRGB,
    Vbgr = CAIRO_SUBPIXEL_ORDER_VBGR,
  };

  enum class HintStyle : std::underlying_type_t<cairo_hint_style_t> { // NOLINT(performance-enum-size)
    Default = CAIRO_HINT_STYLE_DEFAULT,
    None = CAIRO_HINT_STYLE_NONE,
    Slight = CAIRO_HINT_STYLE_SLIGHT,
    Medium = CAIRO_HINT_STYLE_MEDIUM,
    Full = CAIRO_HINT_STYLE_FULL,
  };

  enum class HintMetrics : std::underlying_type_t<cairo_hint_metrics_t> { // NOLINT(performance-enum-size)
    Default = CAIRO_HINT_METRICS_DEFAULT,
    Off = CAIRO_HINT_METRICS_OFF,
    On = CAIRO_HINT_METRICS_ON,
  };

  class FontOptions {
  public:
    FontOptions()
    : m_options(cairo_font_options_create())
    {
    }

    Status status() { return static_cast<Status>(cairo_font_options_status(m_options)); }
    void merge(const FontOptions& other) { cairo_font_options_merge(m_options, other.m_options); }
    unsigned long hash() const { return cairo_font_options_hash(m_options); }

    void set_antialias(Antialias aa) { cairo_font_options_set_antialias(m_options, static_cast<cairo_antialias_t>(aa)); }
    Antialias antialias() const { return static_cast<Antialias>(cairo_font_options_get_antialias(m_options)); }

    void set_subpixel_order(SubpixelOrder so) { cairo_font_options_set_subpixel_order(m_options, static_cast<cairo_subpixel_order_t>(so)); }
    SubpixelOrder subpixel_order() const { return static_cast<SubpixelOrder>(cairo_font_options_get_subpixel_order(m_options)); }

    void set_hint_style(HintStyle hs) { cairo_font_options_set_hint_style(m_options, static_cast<cairo_hint_style_t>(hs)); }
    HintStyle hint_style() const { return static_cast<HintStyle>(cairo_font_options_get_hint_style(m_options)); }

    void set_hint_metrics(HintMetrics hm) { cairo_font_options_set_hint_metrics(m_options, static_cast<cairo_hint_metrics_t>(hm)); }
    HintMetrics hint_metrics() const { return static_cast<HintMetrics>(cairo_font_options_get_hint_metrics(m_options)); }

    void set_variations(const char* variations) { cairo_font_options_set_variations(m_options, variations); }
    const char* variations() { return cairo_font_options_get_variations(m_options); }

    bool operator==(const FontOptions& other) const { return cairo_font_options_equal(m_options, other.m_options) != 0; }

  private:
    friend class Context;
    friend class ScaledFont;
    friend class Surface;
    details::CopyableHandle<cairo_font_options_t, cairo_font_options_copy, cairo_font_options_destroy> m_options;
  };

  enum class FontType : std::underlying_type_t<cairo_font_type_t> { // NOLINT(performance-enum-size)
    Toy = CAIRO_FONT_TYPE_TOY,
    Ft = CAIRO_FONT_TYPE_FT,
    Win32 = CAIRO_FONT_TYPE_WIN32,
    Quartz = CAIRO_FONT_TYPE_QUARTZ,
    User = CAIRO_FONT_TYPE_USER
  };

  class ToyFontFace;

  class FontFace {
  public:

    Status status() { return static_cast<Status>(cairo_font_face_status(m_font)); }
    FontType type() { return static_cast<FontType>(cairo_font_face_get_type(m_font)); }

    inline ToyFontFace& as_toy();

  protected:
    FontFace(cairo_font_face_t* font, [[maybe_unused]] details::IncreaseReferenceType incr)
    : m_font(font, details::IncreaseReference)
    {
    }

    struct DerivedFontType {};
    static constexpr DerivedFontType DerivedFont = {};

    FontFace(cairo_font_face_t* font, [[maybe_unused]] DerivedFontType derived)
    : m_font(font)
    {
    }

    cairo_font_face_t* raw() { return m_font; }

  private:
    friend class Context;
    friend class ScaledFont;

    details::Handle<cairo_font_face_t, cairo_font_face_reference, cairo_font_face_destroy> m_font;
  };

  struct TextGlyphs {
    Status result = Status::Success;
    std::vector<glyph> glyphs;
    std::vector<text_cluster> clusters;
    TextClusterFlags flags = TextClusterFlags::None;
  };

  class ToyFontFace : public FontFace {
  public:
    ToyFontFace(const char* family, FontSlant slant, FontWeight weight)
    : FontFace(cairo_toy_font_face_create(family, static_cast<cairo_font_slant_t>(slant), static_cast<cairo_font_weight_t>(weight)), DerivedFont)
    {
    }

    const char* family() { return cairo_toy_font_face_get_family(raw()); }
    FontSlant slant() { return static_cast<FontSlant>(cairo_toy_font_face_get_slant(raw())); }
    FontWeight weight() { return static_cast<FontWeight>(cairo_toy_font_face_get_weight(raw())); }
  };

  inline ToyFontFace& FontFace::as_toy() {
    assert(type() == FontType::Toy);
    return static_cast<ToyFontFace&>(*this); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
  }


  class ScaledFont {
  public:
    ScaledFont(FontFace& font, const Matrix& font_matrix, const Matrix& ctm, const FontOptions& options)
    : m_font(cairo_scaled_font_create(font.m_font, font_matrix, ctm, options.m_options))
    {
    }

    Status status() { return static_cast<Status>(cairo_scaled_font_status(m_font)); }
    FontType type() { return static_cast<FontType>(cairo_scaled_font_get_type(m_font)); }

    FontExtents font_extents() { FontExtents extents; cairo_scaled_font_extents(m_font, &extents); return extents; }
    TextExtents text_extents(const char* utf8) { TextExtents extents; cairo_scaled_font_text_extents(m_font, utf8, &extents); return extents; }
    TextExtents glyph_extents(const glyph* glyphs, int num_glyphs) { TextExtents extents; cairo_scaled_font_glyph_extents(m_font, glyphs, num_glyphs, &extents); return extents; }
    template<typename T>
    TextExtents glyph_extents(const T& glyphs) { TextExtents extents; cairo_scaled_font_glyph_extents(m_font, std::data(glyphs), static_cast<int>(std::size(glyphs)), &extents); return extents; }

    TextGlyphs text_to_glyphs(double x, double y, const char* utf8, int utf8_len)
    {
      glyph* glyphs = nullptr;
      int num_glyphs = 0;
      text_cluster* clusters = nullptr;
      int num_clusters = 0;
      auto flags = cairo_text_cluster_flags_t(0);

      TextGlyphs ret;

      ret.result = static_cast<enum Status>(cairo_scaled_font_text_to_glyphs(m_font, x, y, utf8, utf8_len, &glyphs, &num_glyphs, &clusters, &num_clusters, &flags));

      if (ret.result == Status::Success) {
        ret.glyphs.assign(glyphs, glyphs + num_glyphs);
        cairo_glyph_free(glyphs);
        ret.clusters.assign(clusters, clusters + num_clusters);
        cairo_text_cluster_free(clusters);
        ret.flags = static_cast<TextClusterFlags>(flags);
      }

      return ret;
    }

    FontFace font_face() { return { cairo_scaled_font_get_font_face(m_font), details::IncreaseReference }; }
    Matrix font_matrix() { Matrix m; cairo_scaled_font_get_font_matrix(m_font, m); return m; }
    Matrix ctm() { Matrix m; cairo_scaled_font_get_ctm(m_font, m); return m; }
    Matrix scale_matrix() { Matrix m; cairo_scaled_font_get_scale_matrix(m_font, m); return m; }
    FontOptions font_options() { FontOptions opt; cairo_scaled_font_get_font_options(m_font, opt.m_options); return opt; }

  private:
    ScaledFont(cairo_scaled_font_t* font, [[maybe_unused]] details::IncreaseReferenceType incr)
    : m_font(font, details::IncreaseReference)
    {
    }

    friend class Context;
    details::Handle<cairo_scaled_font_t, cairo_scaled_font_reference, cairo_scaled_font_destroy> m_font;
  };

  /*
   * path
   */

  enum class PathDataType : std::underlying_type_t<cairo_path_data_type_t> { // NOLINT(performance-enum-size)
    MoveTo = CAIRO_PATH_MOVE_TO,
    LineTo = CAIRO_PATH_LINE_TO,
    CurveTo = CAIRO_PATH_CURVE_TO,
    ClosePath = CAIRO_PATH_CLOSE_PATH,
  };

  using PathData = cairo_path_data_t;

  class PathElement {
  public:
    PathDataType type() const { return static_cast<PathDataType>(m_data[0].header.type);  }
    int length() const { return m_data[0].header.length; }

    Vec2F point(int i) const
    {
      assert(i < length());
      return { m_data[i].point.x, m_data[i].point.y };
    }

    Vec2F operator[](int i) const { return point(i); }

    constexpr bool operator==(const PathElement& other) const noexcept { return m_data == other.m_data; }
    constexpr bool operator!=(const PathElement& other) const noexcept { return m_data != other.m_data; }

  private:
    PathElement(const PathData* data)
    : m_data(data)
    {
    }

    friend class PathIterator;
    const PathData* m_data = nullptr;
  };

  class PathIterator {
  public:
    using value_type = PathElement;
    using difference_type = int;
    using reference = value_type;
    using pointer = value_type;
    using iterator_category = std::forward_iterator_tag;

    reference operator*() { return m_data; }
    pointer operator->() { return m_data; }

    PathIterator& operator++() { m_data += m_data[0].header.length; return *this; }
    PathIterator operator++(int) { PathIterator copy = *this; ++copy; return copy; }

    constexpr bool operator==(const PathIterator& other) const noexcept { return m_data == other.m_data; }
    constexpr bool operator!=(const PathIterator& other) const noexcept { return m_data != other.m_data; }

  private:
    friend class Path;
    PathIterator(const PathData* data)
    : m_data(data)
    {
    }

    const PathData* m_data = nullptr;
  };

  class Path {
  public:
    Status status() const { return static_cast<Status>(m_path.get()->status); };

    PathIterator begin() const { return m_path.get()->data; }
    PathIterator end() const { return m_path.get()->data + m_path.get()->num_data; }

  private:
    Path(cairo_path* p)
    : m_path(p)
    {
    }

    friend class Context;
    friend class MeshPattern;
    details::NonCopyableHandle<cairo_path, cairo_path_destroy> m_path;
  };

  /*
   * pattern
   */

  enum class PatternType : std::underlying_type_t<cairo_pattern_type_t> { // NOLINT(performance-enum-size)
    Solid = CAIRO_PATTERN_TYPE_SOLID,
    Surface = CAIRO_PATTERN_TYPE_SURFACE,
    Linear = CAIRO_PATTERN_TYPE_LINEAR,
    Radial = CAIRO_PATTERN_TYPE_RADIAL,
    Mesh = CAIRO_PATTERN_TYPE_MESH,
    RasterSource = CAIRO_PATTERN_TYPE_RASTER_SOURCE,
  };

  enum class Extend : std::underlying_type_t<cairo_extend_t> { // NOLINT(performance-enum-size)
    None = CAIRO_EXTEND_NONE,
    Repeat = CAIRO_EXTEND_REPEAT,
    Reflect = CAIRO_EXTEND_REFLECT,
    Pad = CAIRO_EXTEND_PAD,
  };

  enum class Filter : std::underlying_type_t<cairo_filter_t> { // NOLINT(performance-enum-size)
    Fast = CAIRO_FILTER_FAST,
    Good = CAIRO_FILTER_GOOD,
    Best = CAIRO_FILTER_BEST,
    Nearest = CAIRO_FILTER_NEAREST,
    Bilinear = CAIRO_FILTER_BILINEAR,
    Gaussian = CAIRO_FILTER_GAUSSIAN,
  };

  class Pattern {
  public:
    Status status() { return static_cast<Status>(cairo_pattern_status(m_pattern)); }
    PatternType type() { return static_cast<PatternType>(cairo_pattern_get_type(m_pattern)); }

    void set_matrix(const Matrix& m) { cairo_pattern_set_matrix(m_pattern, m); }
    Matrix matrix() { Matrix m; cairo_pattern_get_matrix(m_pattern, m); return m; }

    void set_extend(Extend e) { cairo_pattern_set_extend(m_pattern, static_cast<cairo_extend_t>(e)); }
    Extend extend() { return static_cast<Extend>(cairo_pattern_get_extend(m_pattern)); }

    void set_filter(Filter f) { cairo_pattern_set_filter(m_pattern, static_cast<cairo_filter_t>(f)); }
    Filter filter() { return static_cast<Filter>(cairo_pattern_get_filter(m_pattern)); }

  protected:
    Pattern(cairo_pattern_t* pat)
    : m_pattern(pat)
    {
    }

    Pattern(cairo_pattern_t* pat, [[maybe_unused]] details::IncreaseReferenceType incr)
    : m_pattern(pat, details::IncreaseReference)
    {
    }

    cairo_pattern_t* raw() { return m_pattern; }

  private:
    friend class Context;
    details::Handle<cairo_pattern_t, cairo_pattern_reference, cairo_pattern_destroy> m_pattern;
  };

  /*
   * device
   */

  enum class DeviceType : std::underlying_type_t<cairo_device_type_t> { // NOLINT(performance-enum-size)
    Drm = CAIRO_DEVICE_TYPE_DRM,
    Gl = CAIRO_DEVICE_TYPE_GL,
    Script = CAIRO_DEVICE_TYPE_SCRIPT,
    Xcb = CAIRO_DEVICE_TYPE_XCB,
    Xlib = CAIRO_DEVICE_TYPE_XLIB,
    Xml = CAIRO_DEVICE_TYPE_XML,
    Cogl = CAIRO_DEVICE_TYPE_COGL,
    Win32 = CAIRO_DEVICE_TYPE_WIN32,

    Invalid = CAIRO_DEVICE_TYPE_INVALID,
  };

  class Device {
  public:
    DeviceType type() { return static_cast<DeviceType>(cairo_device_get_type(m_device)); }
    Status status() { return static_cast<Status>(cairo_device_status(m_device)); }

    Status acquire() { return static_cast<Status>(cairo_device_acquire(m_device)); }
    void release() { cairo_device_release(m_device); }

    void flush() { cairo_device_flush(m_device); }
    void finish() { cairo_device_finish(m_device); }

  private:
    Device(cairo_device_t* dev, [[maybe_unused]] details::IncreaseReferenceType incr)
    : m_device(dev, details::IncreaseReference)
    {
    }

    friend class Surface;
    details::Handle<cairo_device_t, cairo_device_reference, cairo_device_destroy> m_device;
  };

  /*
   * surface
   */

  enum class SurfaceType : std::underlying_type_t<cairo_surface_type_t> { // NOLINT(performance-enum-size)
    Image = CAIRO_SURFACE_TYPE_IMAGE,
    Pdf = CAIRO_SURFACE_TYPE_PDF,
    Ps = CAIRO_SURFACE_TYPE_PS,
    Xlib = CAIRO_SURFACE_TYPE_XLIB,
    Xcb = CAIRO_SURFACE_TYPE_XCB,
    Glitz = CAIRO_SURFACE_TYPE_GLITZ,
    Quartz = CAIRO_SURFACE_TYPE_QUARTZ,
    Win32 = CAIRO_SURFACE_TYPE_WIN32,
    Beos = CAIRO_SURFACE_TYPE_BEOS,
    Directfb = CAIRO_SURFACE_TYPE_DIRECTFB,
    Svg = CAIRO_SURFACE_TYPE_SVG,
    Os2 = CAIRO_SURFACE_TYPE_OS2,
    Win32Printing = CAIRO_SURFACE_TYPE_WIN32_PRINTING,
    QuartzImage = CAIRO_SURFACE_TYPE_QUARTZ_IMAGE,
    Script = CAIRO_SURFACE_TYPE_SCRIPT,
    Qt = CAIRO_SURFACE_TYPE_QT,
    Recording = CAIRO_SURFACE_TYPE_RECORDING,
    Vg = CAIRO_SURFACE_TYPE_VG,
    Gl = CAIRO_SURFACE_TYPE_GL,
    Drm = CAIRO_SURFACE_TYPE_DRM,
    Tee = CAIRO_SURFACE_TYPE_TEE,
    Xml = CAIRO_SURFACE_TYPE_XML,
    Skia = CAIRO_SURFACE_TYPE_SKIA,
    Subsurface = CAIRO_SURFACE_TYPE_SUBSURFACE,
    Cogl = CAIRO_SURFACE_TYPE_COGL,
  };

  class Surface {
  public:
    Status status() { return static_cast<Status>(cairo_surface_status(m_surface)); }
    SurfaceType type() { return static_cast<SurfaceType>(cairo_surface_get_type(m_surface)); }
    Content content() { return static_cast<Content>(cairo_surface_get_content(m_surface)); }

    Surface create_similar(Content cnt, int width, int height) { return cairo_surface_create_similar(m_surface, static_cast<cairo_content_t>(cnt), width, height); }
    Surface create_similar(Content cnt, Vec2I size) { return cairo_surface_create_similar(m_surface, static_cast<cairo_content_t>(cnt), size.x, size.y); }
    Surface create_similar_image(Format fmt, int width, int height) { return cairo_surface_create_similar_image(m_surface, static_cast<cairo_format_t>(fmt), width, height); }
    Surface create_similar_image(Format fmt, Vec2I size) { return cairo_surface_create_similar_image(m_surface, static_cast<cairo_format_t>(fmt), size.x, size.y); }
    Surface create_for_rectangle(double x, double y, double width, double height) { return cairo_surface_create_for_rectangle(m_surface, x, y, width, height); }
    Surface create_for_rectangle(RectF rectangle) { return cairo_surface_create_for_rectangle(m_surface, rectangle.x, rectangle.y, rectangle.w, rectangle.h); }

    void finish() { cairo_surface_finish(m_surface); }

    Device device() { return { cairo_surface_get_device(m_surface), details::IncreaseReference };  }

#if CAIRO_HAS_PNG_FUNCTIONS
    Status write_to_png(const char* filename) { return static_cast<Status>(cairo_surface_write_to_png(m_surface, filename)); }
    Status write_to_png(const std::filesystem::path& filename) { return static_cast<Status>(cairo_surface_write_to_png(m_surface, filename.string().c_str())); }
#endif

    FontOptions font_options() { FontOptions opt; cairo_surface_get_font_options(m_surface, opt.m_options); return opt; };

    void flush() { cairo_surface_flush(m_surface); }
    void mark_dirty() { cairo_surface_mark_dirty(m_surface); }
    void mark_dirty_rectangle(int x, int y, int width, int height) { cairo_surface_mark_dirty_rectangle(m_surface, x, y, width, height); }
    void mark_dirty_rectangle(RectI rectangle) { cairo_surface_mark_dirty_rectangle(m_surface, rectangle.x, rectangle.y, rectangle.w, rectangle.h); }

    void set_device_scale(double x_scale, double y_scale) { cairo_surface_set_device_scale(m_surface, x_scale, y_scale); }
    void set_device_scale(Vec2F scale) { cairo_surface_set_device_scale(m_surface, scale.x, scale.y); }
    Vec2F device_scale() { Vec2F scale; cairo_surface_get_device_scale(m_surface, &scale.x, &scale.y); return scale; }
    void set_device_offset(double x_offset, double y_offset) { cairo_surface_set_device_offset(m_surface, x_offset, y_offset); }
    void set_device_offset(Vec2F offset) { cairo_surface_set_device_offset(m_surface, offset.x, offset.y); }
    Vec2F device_offset() { Vec2F offset; cairo_surface_get_device_offset(m_surface, &offset.x, &offset.y); return offset; }
    void set_fallback_resolution(double x_pixels_per_inch, double y_pixels_per_inch) { cairo_surface_set_fallback_resolution(m_surface, x_pixels_per_inch, y_pixels_per_inch); }
    void set_fallback_resolution(Vec2F pixels_per_inch) { cairo_surface_set_fallback_resolution(m_surface, pixels_per_inch.x, pixels_per_inch.y); }
    Vec2F fallback_resolution() { Vec2F pixels_per_inch; cairo_surface_get_fallback_resolution(m_surface, &pixels_per_inch.x, &pixels_per_inch.y); return pixels_per_inch; }

    void copy_page() { cairo_surface_copy_page(m_surface); }
    void show_page() { cairo_surface_show_page(m_surface); }
    bool has_show_text_glyphs() { return cairo_surface_has_show_text_glyphs(m_surface) != 0; }

  protected:
    Surface(cairo_surface_t* surf)
    : m_surface(surf)
    {
    }

    Surface(cairo_surface_t* surf, [[maybe_unused]] details::IncreaseReferenceType incr)
    : m_surface(surf, details::IncreaseReference)
    {
    }

    cairo_surface_t* raw() { return m_surface; }

  private:
    friend class Context;
    friend class SurfacePattern;
    details::Handle<cairo_surface_t, cairo_surface_reference, cairo_surface_destroy> m_surface;
  };

  /*
   * context
   */

  struct DashArray {
    std::vector<double> dashes;
    double offset = 0.0;
  };

  class Context {
  public:
    Context(Surface& surf)
    : m_context(cairo_create(surf.m_surface))
    {
    }

    Status status() { return static_cast<Status>(cairo_status(m_context)); }
    Surface target() { return { cairo_get_target(m_context), details::IncreaseReference }; }

    void save() { cairo_save(m_context); }
    void restore() { cairo_restore(m_context); }

    void push_group() { cairo_push_group(m_context); }
    void push_group_with_content(Content c) { cairo_push_group_with_content(m_context, static_cast<cairo_content_t>(c)); }
    Pattern pop_group() { return cairo_pop_group(m_context); }
    void pop_group_to_source() { cairo_pop_group_to_source(m_context); }
    Surface group_target() { return { cairo_get_group_target(m_context), details::IncreaseReference }; }

    // modify state

    void set_compositing_operator(Operator op) { cairo_set_operator(m_context, static_cast<cairo_operator_t>(op)); }
    Operator compositing_operator() { return static_cast<Operator>(cairo_get_operator(m_context)); }
    void set_source(Pattern& pat) { cairo_set_source(m_context, pat.m_pattern); }
    Pattern source() { return { cairo_get_source(m_context), details::IncreaseReference }; }
    void set_source(Surface& surf, double x, double y) { cairo_set_source_surface(m_context, surf.m_surface, x, y); }
    void set_source(Surface& surf, Vec2F origin) { cairo_set_source_surface(m_context, surf.m_surface, origin.x, origin.y); }
    void set_source_rgb(double red, double green, double blue) { cairo_set_source_rgb(m_context, red, green, blue); }
    void set_source_rgba(double red, double green, double blue, double alpha) { cairo_set_source_rgba(m_context, red, green, blue, alpha); }
    void set_source_color(Color col) { cairo_set_source_rgba(m_context, col.r, col.g, col.b, col.a); }
    void set_tolerance(double tolerance) { cairo_set_tolerance(m_context, tolerance); }
    double tolerance() { return cairo_get_tolerance(m_context); }

    void set_antialias(Antialias aa) { cairo_set_antialias(m_context, static_cast<cairo_antialias_t>(aa)); }
    Antialias antialias() { return static_cast<Antialias>(cairo_get_antialias(m_context)); }

    void set_fill_rule(FillRule fr) { cairo_set_fill_rule(m_context, static_cast<cairo_fill_rule_t>(fr)); }
    FillRule fill_rule() { return static_cast<FillRule>(cairo_get_fill_rule(m_context)); }
    void set_line_width(double width) { cairo_set_line_width(m_context, width); }
    double line_width() { return cairo_get_line_width(m_context); }
    void set_line_cap(LineCap lc) { cairo_set_line_cap(m_context, static_cast<cairo_line_cap_t>(lc)); }
    LineCap line_cap() { return static_cast<LineCap>(cairo_get_line_cap(m_context)); }
    void set_line_join(LineJoin lj) { cairo_set_line_join(m_context, static_cast<cairo_line_join_t>(lj)); }
    LineJoin line_join() { return static_cast<LineJoin>(cairo_get_line_join(m_context)); }

    void set_dash(const double* dashes, int num_dashes, double offset) { cairo_set_dash(m_context, dashes, num_dashes, offset); };
    template<typename T>
    void set_dash(const T& dashes, double offset) { cairo_set_dash(m_context, std::data(dashes), static_cast<int>(std::size(dashes)), offset); }
    int dash_count() { return cairo_get_dash_count(m_context); }
    DashArray dash()
    {
      const int count = dash_count();
      DashArray array;
      array.dashes.resize(count);
      cairo_get_dash(m_context, array.dashes.data(), &array.offset);
      return array;
    }

    void set_miter_limit(double limit) { cairo_set_miter_limit(m_context, limit); }
    double mitter_limit() { return cairo_get_miter_limit(m_context); }

    Context& translate(double tx, double ty) { cairo_translate(m_context, tx, ty); return *this; };
    Context& translate(Vec2F translation) { cairo_translate(m_context, translation.x, translation.y); return *this; };
    Context& scale(double sx, double sy) { cairo_scale(m_context, sx, sy); return *this; }
    Context& scale(Vec2F scale) { cairo_scale(m_context, scale.x, scale.y); return *this; }
    Context& rotate(double angle) { cairo_rotate(m_context, angle); return *this; }
    Context& transform(const Matrix& m) { cairo_transform(m_context, m); return *this; }

    void set_matrix(const Matrix& m) { cairo_set_matrix(m_context, m); }
    Matrix matrix() { Matrix m; cairo_get_matrix(m_context, m); return m; }
    void identity_matrix() { cairo_identity_matrix(m_context); }

    Vec2F user_to_device(double x, double y) { cairo_user_to_device(m_context, &x, &y); return { x, y }; }
    Vec2F user_to_device(Vec2F point) { cairo_user_to_device(m_context, &point.x, &point.y); return point; }
    Vec2F user_to_device_distance(double dx, double dy) { cairo_user_to_device_distance(m_context, &dx, &dy); return { dx, dy }; }
    Vec2F user_to_device_distance(Vec2F distance) { cairo_user_to_device_distance(m_context, &distance.x, &distance.y); return distance; }
    Vec2F device_to_user(double x, double y) { cairo_device_to_user(m_context, &x, &y); return { x, y }; }
    Vec2F device_to_user(Vec2F point) { cairo_device_to_user(m_context, &point.x, &point.y); return point; }
    Vec2F device_to_user_distance(double dx, double dy) { cairo_device_to_user_distance(m_context, &dx, &dy); return { dx, dy }; }
    Vec2F device_to_user_distance(Vec2F distance) { cairo_device_to_user_distance(m_context, &distance.x, &distance.y); return distance; }

    // path creation function

    Context& new_path() { cairo_new_path(m_context); return *this; }
    Context& new_sub_path() { cairo_new_sub_path(m_context); return *this; }
    Context& move_to(double x, double y) { cairo_move_to(m_context, x, y); return *this; }
    Context& move_to(Vec2F point) { cairo_move_to(m_context, point.x, point.y); return *this; }
    Context& line_to(double x, double y) { cairo_line_to(m_context, x, y); return *this; }
    Context& line_to(Vec2F point) { cairo_line_to(m_context, point.x, point.y); return *this; }
    Context& curve_to(double x1, double y1, double x2, double y2, double x3, double y3) { cairo_curve_to(m_context, x1, y1, x2, y2, x3, y3); return *this; }
    Context& curve_to(Vec2F p1, Vec2F p2, Vec2F p3) { cairo_curve_to(m_context, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y); return *this; }
    Context& arc(double xc, double yc, double radius, double angle1, double angle2) { cairo_arc(m_context, xc, yc, radius, angle1, angle2); return *this; }
    Context& arc(Vec2F center, double radius, double angle1, double angle2) { cairo_arc(m_context, center.x, center.y, radius, angle1, angle2); return *this; }
    Context& arc_negative(double xc, double yc, double radius, double angle1, double angle2) { cairo_arc_negative(m_context, xc, yc, radius, angle1, angle2); return *this; }
    Context& arc_negative(Vec2F center, double radius, double angle1, double angle2) { cairo_arc_negative(m_context, center.x, center.y, radius, angle1, angle2); return *this; }
    Context& rel_move_to(double dx, double dy) { cairo_rel_move_to(m_context, dx, dy); return *this; }
    Context& rel_move_to(Vec2F d) { cairo_rel_move_to(m_context, d.x, d.y); return *this; }
    Context& rel_line_to(double dx, double dy) { cairo_rel_line_to(m_context, dx, dy); return *this; }
    Context& rel_line_to(Vec2F d) { cairo_rel_line_to(m_context, d.x, d.y); return *this; }
    Context& rel_curve_to(double dx1, double dy1, double dx2, double dy2, double dx3, double dy3) { cairo_rel_curve_to(m_context, dx1, dy1, dx2, dy2, dx3, dy3); return *this; }
    Context& rel_curve_to(Vec2F d1, Vec2F d2, Vec2F d3) { cairo_curve_to(m_context, d1.x, d1.y, d2.x, d2.y, d3.x, d3.y); return *this; }
    Context& rectangle(double x, double y, double w, double h) { cairo_rectangle(m_context, x, y, w, h); return *this; }
    Context& rectangle(const RectF& r) { cairo_rectangle(m_context, r.x, r.y, r.w, r.h); return *this; }
    void close_path() { cairo_close_path(m_context); }

    RectF path_extents()
    {
      double x1 = 0.0;
      double y1 = 0.0;
      double x2 = 0.0;
      double y2 = 0.0;
      cairo_path_extents(m_context, &x1, &y1, &x2, &y2);
      return { x1, y1, x2 - x1, y2 -y1 };
    }

    bool has_current_point() { return cairo_has_current_point(m_context) != 0; }
    Vec2F current_point() { Vec2F point; cairo_get_current_point(m_context, &point.x, &point.y); return point; }

    Path copy_path() { return cairo_copy_path(m_context); }
    Path copy_path_flat() { return cairo_copy_path_flat(m_context); }
    void append_path(const Path& p) { cairo_append_path(m_context, p.m_path); }

    // painting

    void paint() { cairo_paint(m_context); }
    void paint_with_alpha(double alpha) { cairo_paint_with_alpha(m_context, alpha); }
    void mask(Pattern& pat) { cairo_mask(m_context, pat.m_pattern); }
    void mask(Surface& surf, double surface_x, double surface_y) { cairo_mask_surface(m_context, surf.m_surface, surface_x, surface_y); }
    void mask(Surface& surf, Vec2F origin) { cairo_mask_surface(m_context, surf.m_surface, origin.x, origin.y); }
    void stroke() { cairo_stroke(m_context); }
    void stroke_preserve() { cairo_stroke_preserve(m_context); }
    void fill() { cairo_fill(m_context); }
    void fill_preserve() { cairo_fill_preserve(m_context); }
    void copy_page() { cairo_copy_page(m_context); }
    void show_page() { cairo_show_page(m_context); }

    // insideness testing

    bool in_stroke(double x, double y) { return cairo_in_stroke(m_context, x, y) != 0; }
    bool in_stroke(Vec2F point) { return cairo_in_stroke(m_context, point.x, point.y) != 0; }
    bool in_fill(double x, double y) { return cairo_in_fill(m_context, x, y) != 0; }
    bool in_fill(Vec2F point) { return cairo_in_fill(m_context, point.x, point.y) != 0; }
    bool in_clip(double x, double y) { return cairo_in_clip(m_context, x, y) != 0; }
    bool in_clip(Vec2F point) { return cairo_in_clip(m_context, point.x, point.y) != 0; }

    // rectangular extents

    RectF stroke_extents()
    {
      double x1 = 0.0;
      double y1 = 0.0;
      double x2 = 0.0;
      double y2 = 0.0;
      cairo_stroke_extents(m_context, &x1, &y1, &x2, &y2);
      return { x1, y1, x2 - x1, y2 -y1 };
    }

    RectF fill_extents()
    {
      double x1 = 0.0;
      double y1 = 0.0;
      double x2 = 0.0;
      double y2 = 0.0;
      cairo_fill_extents(m_context, &x1, &y1, &x2, &y2);
      return { x1, y1, x2 - x1, y2 -y1 };
    }

    // clipping

    void reset_clip() { cairo_reset_clip(m_context); }
    void clip() { cairo_clip(m_context); }
    void clip_preserve() { cairo_clip_preserve(m_context); }
    RectF clip_extents()
    {
      double x1 = 0.0;
      double y1 = 0.0;
      double x2 = 0.0;
      double y2 = 0.0;
      cairo_clip_extents(m_context, &x1, &y1, &x2, &y2);
      return { x1, y1, x2 - x1, y2 -y1 };
    }

    std::vector<RectF> clip_rectangle_list()
    {
      std::unique_ptr<cairo_rectangle_list_t, decltype(&cairo_rectangle_list_destroy)> ptr(cairo_copy_clip_rectangle_list(m_context), cairo_rectangle_list_destroy);

      if (ptr->status != CAIRO_STATUS_SUCCESS) {
        return {};
      }

      std::vector<RectF> list;

      for (int i = 0; i < ptr->num_rectangles; ++i) {
        auto r = ptr->rectangles[i];
        list.push_back({ r.x, r.y, r.width, r.height });
      }

      return list;
    }

    // logical structure tagging functions

    void tag_begin(const char* tag_name, const char* attributes) { cairo_tag_begin(m_context, tag_name, attributes); }
    void tag_end(const char* tag_name) { cairo_tag_end(m_context, tag_name); }

    // text

    void select_font_face(const char* family, FontSlant slant, FontWeight weight) { cairo_select_font_face(m_context, family, static_cast<cairo_font_slant_t>(slant), static_cast<cairo_font_weight_t>(weight)); }
    void set_font_size(double size) { cairo_set_font_size(m_context, size); }
    void set_font_matrix(const Matrix& m) { cairo_set_font_matrix(m_context, m); }
    Matrix font_matrix() { Matrix m; cairo_get_font_matrix(m_context, m); return m; }
    void set_font_options(const FontOptions& opt) { cairo_set_font_options(m_context, opt.m_options); }
    FontOptions font_options() { FontOptions opt; cairo_get_font_options(m_context, opt.m_options); return opt; }
    void set_font_face(FontFace& font) { cairo_set_font_face(m_context, font.m_font); }
    FontFace font_face() { return { cairo_get_font_face(m_context), details::IncreaseReference }; }
    void set_scaled_font(ScaledFont& font) { cairo_set_scaled_font(m_context, font.m_font); }
    ScaledFont scaled_font() { return { cairo_get_scaled_font(m_context), details::IncreaseReference }; }

    void show_text(const char* utf8) { cairo_show_text(m_context, utf8); }
    void show_glyphs(const glyph* glyphs, int num_glyphs) { cairo_show_glyphs(m_context, glyphs, num_glyphs); }
    template<typename T>
    void show_glyphs(const T& glyphs) { cairo_show_glyphs(m_context, std::data(glyphs), static_cast<int>(std::size(glyphs))); }
    void show_text_glyphs(const char* utf8, int utf8_len, const glyph* glyphs, int num_glyphs, const text_cluster* clusters, int num_clusters, TextClusterFlags cluster_flags = TextClusterFlags::None) { cairo_show_text_glyphs(m_context, utf8, utf8_len, glyphs, num_glyphs, clusters, num_clusters, static_cast<cairo_text_cluster_flags_t>(cluster_flags)); }
    template<typename T, typename U>
    void show_text_glyphs(std::string_view utf8, const T& glyphs, const U& clusters, TextClusterFlags cluster_flags = TextClusterFlags::None) { cairo_show_text_glyphs(m_context, std::data(utf8), static_cast<int>(std::size(utf8)), std::data(glyphs), static_cast<int>(std::size(glyphs)), std::data(clusters), static_cast<int>(std::size(clusters)), cluster_flags); }
    void text_path(const char* utf8) { cairo_text_path(m_context, utf8); }
    void glyph_path(const glyph* glyphs, int num_glyphs) { cairo_glyph_path(m_context, glyphs, num_glyphs); }
    template<typename T>
    void glyph_path(const T& glyphs) { cairo_glyph_path(m_context, std::data(glyphs), static_cast<int>(std::size(glyphs))); }

    TextExtents text_extents(const char* utf8) { TextExtents extents; cairo_text_extents(m_context, utf8, &extents); return extents; }
    TextExtents glyph_extents(const glyph* glyphs, int num_glyphs) { TextExtents extents; cairo_glyph_extents(m_context, glyphs, num_glyphs, &extents); return extents; }
    template<typename T>
    TextExtents glyph_extents(const T& glyphs) { TextExtents extents; cairo_glyph_extents(m_context, std::data(glyphs), static_cast<int>(std::size(glyphs)), &extents); return extents; }
    FontExtents font_extents() { FontExtents extents; cairo_font_extents(m_context, &extents); return extents; }

  private:
    details::Handle<cairo_t, cairo_reference, cairo_destroy> m_context;
  };

  class Subcontext {
  public:
    Subcontext(Context& ctx)
    : m_context(&ctx)
    {
      m_context->save();
    }

    Subcontext(const Subcontext&) = delete;
    Subcontext(Subcontext&&) noexcept = delete;

    ~Subcontext()
    {
      m_context->restore();
    }

    Subcontext& operator=(const Subcontext&) = delete;
    Subcontext& operator=(Subcontext&&) noexcept = delete;

  private:
    Context* m_context = nullptr;
  };

  /*
   * patterns
   */

  class SolidPattern : public Pattern {
  public:
    static SolidPattern create_rgb(double red, double green, double blue) { return cairo_pattern_create_rgb(red, green, blue); }
    static SolidPattern create_rgba(double red, double green, double blue, double alpha) { return cairo_pattern_create_rgba(red, green, blue, alpha); }
    static SolidPattern create_color(Color col) { return cairo_pattern_create_rgba(col.r, col.g, col.b, col.a); }

    Color color() { Color col; cairo_pattern_get_rgba(raw(), &col.r, &col.g, &col.b, &col.a); return col; }


  private:
    SolidPattern(cairo_pattern_t* pat)
    : Pattern(pat)
    {
    }
  };

  class SurfacePattern : public Pattern {
  public:
    static SurfacePattern create(Surface& surf) { return cairo_pattern_create_for_surface(surf.m_surface); }

    Surface surface() { cairo_surface_t* surface = nullptr; cairo_pattern_get_surface(raw(), &surface); return { surface, details::IncreaseReference }; }

  private:
    SurfacePattern(cairo_pattern_t* pat)
    : Pattern(pat)
    {
    }
  };

  class GradientPattern : public Pattern {
  public:

    void add_color_stop_rgb(double offset, double red, double green, double blue) { cairo_pattern_add_color_stop_rgb(raw(), offset, red, green, blue); }
    void add_color_stop_rgba(double offset, double red, double green, double blue, double alpha) { cairo_pattern_add_color_stop_rgba(raw(), offset, red, green, blue, alpha); }
    void add_color_stop_color(double offset, Color col) { cairo_pattern_add_color_stop_rgba(raw(), offset, col.r, col.g, col.b, col.a); }

    std::pair<double, Color> color_stop(int index)
    {
      double offset = 0.0;
      Color col;
      [[maybe_unused]] auto result = cairo_pattern_get_color_stop_rgba(raw(), index, &offset, &col.r, &col.g, &col.b, &col.a);
      assert(result == CAIRO_STATUS_SUCCESS);
      return { offset, col };
    }

    int color_stop_count()
    {
      int count = 0;
      [[maybe_unused]] auto result = cairo_pattern_get_color_stop_count(raw(), &count);
      assert(result == CAIRO_STATUS_SUCCESS);
      return count;
    }

  protected:
    GradientPattern(cairo_pattern_t* pat)
    : Pattern(pat)
    {
    }
  };

  class LinearGradientPattern : public GradientPattern {
  public:
    static LinearGradientPattern create(double x0, double y0, double x1, double y1) { return cairo_pattern_create_linear(x0, y0, x1, y1); }
    static LinearGradientPattern create(Vec2F p0, Vec2F p1) { return cairo_pattern_create_linear(p0.x, p0.y, p1.x, p1.y); }

    std::pair<Vec2F, Vec2F> linear_points()
    {
      Vec2F p0;
      Vec2F p1;
      [[maybe_unused]] auto result = cairo_pattern_get_linear_points(raw(), &p0.x, &p0.y, &p1.x, &p1.y);
      assert(result == CAIRO_STATUS_SUCCESS);
      return { p0, p1 };
    }

  private:
    LinearGradientPattern(cairo_pattern_t* pat)
    : GradientPattern(pat)
    {
    }
  };

  class RadialGradientPattern : public GradientPattern {
  public:
    static RadialGradientPattern create(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1) { return cairo_pattern_create_radial(cx0, cy0, radius0, cx1, cy1, radius1); }
    static RadialGradientPattern create(Vec2F center0, double radius0, Vec2F center1, double radius1) { return cairo_pattern_create_radial(center0.x, center0.y, radius0, center1.x, center1.y, radius1); }

    std::tuple<Vec2F, double, Vec2F, double> radial_circles()
    {
      Vec2F center0;
      double radius0 = 0.0;
      Vec2F center1;
      double radius1 = 0.0;
      [[maybe_unused]] auto result = cairo_pattern_get_radial_circles(raw(), &center0.x, &center0.y, &radius0, &center1.x, &center1.y, &radius1);
      assert(result == CAIRO_STATUS_SUCCESS);
      return { center0, radius0, center1, radius1 };
    }

  private:
    RadialGradientPattern(cairo_pattern_t* pat)
    : GradientPattern(pat)
    {
    }
  };

  class MeshPattern : public Pattern {
  public:
    static MeshPattern create() { return cairo_pattern_create_mesh(); }

    void begin_patch() { cairo_mesh_pattern_begin_patch(raw()); }
    void end_patch() { cairo_mesh_pattern_end_patch(raw()); }
    void move_to(double x, double y) { cairo_mesh_pattern_move_to(raw(), x, y); }
    void move_to(Vec2F point) { cairo_mesh_pattern_move_to(raw(), point.x, point.y); }
    void line_to(double x, double y) { cairo_mesh_pattern_line_to(raw(), x, y); }
    void line_to(Vec2F point) { cairo_mesh_pattern_line_to(raw(), point.x, point.y); }
    void curve_to(double x1, double y1, double x2, double y2, double x3, double y3) { cairo_mesh_pattern_curve_to(raw(), x1, y1, x2, y2, x3, y3); }
    void curve_to(Vec2F p1, Vec2F p2, Vec2F p3) { cairo_mesh_pattern_curve_to(raw(), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y); }

    void set_control_point(unsigned point_num, double x, double y) { cairo_mesh_pattern_set_control_point(raw(), point_num, x, y); }
    void set_control_point(unsigned point_num, Vec2F point) { cairo_mesh_pattern_set_control_point(raw(), point_num, point.x, point.y); }
    void set_corner_color_rgb(unsigned corner_num, double red, double green, double blue) { cairo_mesh_pattern_set_corner_color_rgb(raw(), corner_num, red, green, blue); }
    void set_corner_color_rgba(unsigned corner_num, double red, double green, double blue, double alpha) { cairo_mesh_pattern_set_corner_color_rgba(raw(), corner_num, red, green, blue, alpha); }
    void set_corner_color(unsigned corner_num, Color col) { cairo_mesh_pattern_set_corner_color_rgba(raw(), corner_num, col.r, col.g, col.b, col.a); }

    unsigned patch_count()
    {
      unsigned count = 0;
      [[maybe_unused]] auto result = cairo_mesh_pattern_get_patch_count(raw(), &count);
      assert(result == CAIRO_STATUS_SUCCESS);
      return count;
    }

    Path path(unsigned patch_num) { return cairo_mesh_pattern_get_path(raw(), patch_num); }

    Color corner_color(unsigned patch_num, unsigned corner_num)
    {
      Color col;
      [[maybe_unused]] auto result = cairo_mesh_pattern_get_corner_color_rgba(raw(), patch_num, corner_num, &col.r, &col.g, &col.b, &col.a);
      assert(result == CAIRO_STATUS_SUCCESS);
      return col;
    }

    Vec2F control_point(unsigned patch_num, unsigned point_num)
    {
      Vec2F point;
      [[maybe_unused]] auto result = cairo_mesh_pattern_get_control_point(raw(), patch_num, point_num, &point.x, &point.y);
      assert(result == CAIRO_STATUS_SUCCESS);
      return point;
    }

  private:
    MeshPattern(cairo_pattern_t* pat)
    : Pattern(pat)
    {
    }
  };

  /*
   * surfaces
   */

  inline int format_stride_for_width(Format fmt, int width) { return cairo_format_stride_for_width(static_cast<cairo_format_t>(fmt), width); }

  class ImageSurface : public Surface {
  public:
    static ImageSurface create(Format fmt, int width, int height) { return cairo_image_surface_create(static_cast<cairo_format_t>(fmt), width, height); }
    static ImageSurface create(Format fmt, Vec2I size) { return cairo_image_surface_create(static_cast<cairo_format_t>(fmt), size.x, size.y); }
    static ImageSurface create_for_data(unsigned char* data, Format fmt, int width, int height, int stride) { return cairo_image_surface_create_for_data(data, static_cast<cairo_format_t>(fmt), width, height, stride); }
    static ImageSurface create_for_data(unsigned char* data, Format fmt, Vec2I size, int stride) { return cairo_image_surface_create_for_data(data, static_cast<cairo_format_t>(fmt), size.x, size.y, stride); }

#if CAIRO_HAS_PNG_FUNCTIONS
    static ImageSurface create_from_png(const char* filename) { return cairo_image_surface_create_from_png(filename);  }
    static ImageSurface create_from_png(const std::filesystem::path& filename) { return cairo_image_surface_create_from_png(filename.string().c_str());  }
#endif

    unsigned char* data() { return cairo_image_surface_get_data(raw()); }
    Format format() { return static_cast<Format>(cairo_image_surface_get_format(raw())); }
    int width() { return cairo_image_surface_get_width(raw()); }
    int height() { return cairo_image_surface_get_height(raw()); }
    int stride() { return cairo_image_surface_get_stride(raw()); }


  private:
    ImageSurface(cairo_surface_t* surf)
    : Surface(surf)
    {
    }
  };

  class RecordingSurface : public Surface {
  public:
    static RecordingSurface create(Content cnt, RectF extents) { const cairo_rectangle_t rectangle = { extents.x, extents.y, extents.w, extents.h }; return cairo_recording_surface_create(static_cast<cairo_content_t>(cnt), &rectangle); }

    RectF ink_extents() { RectF extents; cairo_recording_surface_ink_extents(raw(), &extents.x, &extents.y, &extents.w, &extents.h); return extents; }
    std::pair<bool, RectF> extents() { cairo_rectangle_t extents; auto ret = cairo_recording_surface_get_extents(raw(), &extents); return { ret != 0, { extents.x, extents.y, extents.width, extents.height }}; }

  private:
    RecordingSurface(cairo_surface_t* surf)
    : Surface(surf)
    {
    }
  };

#if CAIRO_HAS_PDF_SURFACE

  enum class PdfVersion : std::underlying_type_t<cairo_pdf_version_t> { // NOLINT(performance-enum-size)
    V_1_4,
    V_1_5,
  };

  inline std::string_view to_string(PdfVersion version) { return cairo_pdf_version_to_string(static_cast<cairo_pdf_version_t>(version)); }

  enum class PdfOutlineFlags : std::underlying_type_t<cairo_pdf_outline_flags_t> { // NOLINT(performance-enum-size)
    Open = CAIRO_PDF_OUTLINE_FLAG_OPEN,
    Bold = CAIRO_PDF_OUTLINE_FLAG_BOLD,
    Italic = CAIRO_PDF_OUTLINE_FLAG_ITALIC,
  };

  inline constexpr int PdfOutlineRoot = CAIRO_PDF_OUTLINE_ROOT;

  enum class PdfMetadata : std::underlying_type_t<cairo_pdf_metadata_t> { // NOLINT(performance-enum-size)
    Title = CAIRO_PDF_METADATA_TITLE,
    Author = CAIRO_PDF_METADATA_AUTHOR,
    Subject = CAIRO_PDF_METADATA_SUBJECT,
    Keywords = CAIRO_PDF_METADATA_KEYWORDS,
    Creator = CAIRO_PDF_METADATA_CREATOR,
    Date = CAIRO_PDF_METADATA_CREATE_DATE,
    ModDate = CAIRO_PDF_METADATA_MOD_DATE,
  };

  class PdfSurface : public Surface {
  public:
    static PdfSurface create(const char* filename, double width_in_points, double height_in_points) { return cairo_pdf_surface_create(filename, width_in_points, height_in_points); }
    static PdfSurface create(const std::filesystem::path& filename, double width_in_points, double height_in_points) { return cairo_pdf_surface_create(filename.string().c_str(), width_in_points, height_in_points); }

    void restrict_to_version(PdfVersion version) { cairo_pdf_surface_restrict_to_version(raw(), static_cast<cairo_pdf_version_t>(version)); }

    void set_size(double width_in_points, double height_in_points) {cairo_pdf_surface_set_size(raw(), width_in_points, height_in_points); }
    void set_size(Vec2F size_in_points) {cairo_pdf_surface_set_size(raw(), size_in_points.x, size_in_points.y); }

    void add_outline(int parent_id, const char* utf8, const char* link_attribs, PdfOutlineFlags flags) { cairo_pdf_surface_add_outline(raw(), parent_id, utf8, link_attribs, static_cast<cairo_pdf_outline_flags_t>(flags)); }

    void set_metadata(PdfMetadata metadata, const char* utf8) { cairo_pdf_surface_set_metadata(raw(), static_cast<cairo_pdf_metadata_t>(metadata), utf8); }
    void set_page_label(const char* utf8) { cairo_pdf_surface_set_page_label(raw(), utf8); }
    void set_thumbnail_size(int width, int height) { cairo_pdf_surface_set_thumbnail_size(raw(), width, height); }
    void set_thumbnail_size(Vec2I size) { cairo_pdf_surface_set_thumbnail_size(raw(), size.x, size.y); }

    static std::vector<PdfVersion> versions()
    {
      const cairo_pdf_version_t* raw_versions = nullptr;
      int raw_num_versions = 0;
      cairo_pdf_get_versions(&raw_versions, &raw_num_versions);

      std::vector<PdfVersion> versions;
      versions.reserve(raw_num_versions);

      for (int i = 0; i < raw_num_versions; ++i) {
        versions.push_back(static_cast<PdfVersion>(raw_versions[i]));
      }

      return versions;
    }

  private:
    PdfSurface(cairo_surface_t* surf)
    : Surface(surf)
    {
    }
  };

#endif

  inline void debug_reset_static_data() { cairo_debug_reset_static_data(); }
}

#endif // CAIROPP_H
