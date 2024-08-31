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

    struct increase_reference_type {};
    constexpr increase_reference_type increase_reference = {};

    template<typename T, void (*Destroy)(T*)>
    class basic_handle {
    public:
      basic_handle() = default;

      basic_handle(T* object)
      : m_object(object)
      {
      }

      basic_handle(const basic_handle&) = default;

      basic_handle(basic_handle&& other) noexcept
      : m_object(std::exchange(other.m_object, nullptr))
      {
      }

      ~basic_handle() {
        destroy();
      }

      basic_handle& operator=(const basic_handle&) = default;

      basic_handle& operator=(basic_handle&& other) noexcept
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

    protected:
      T* m_object = nullptr;
    };



    template<typename T, T* (*Reference)(T*), void (*Destroy)(T*)>
    class handle : public basic_handle<T, Destroy> {
    public:
      using basic_handle<T, Destroy>::basic_handle;
      using basic_handle<T, Destroy>::destroy;

      handle(T* object, increase_reference_type)
      : basic_handle<T, Destroy>(object)
      {
        reference();
      }

      handle(const handle& other)
      : basic_handle<T, Destroy>(other.m_object)
      {
        reference();
      }

      handle& operator=(const handle& other)
      {
        if (&other == this) {
          return *this;
        }

        destroy();
        m_object = other.m_object;
        reference();
        return *this;
      }

      void reference()
      {
        if (m_object != nullptr) {
          Reference(m_object);
        }
      }

    private:
      using basic_handle<T, Destroy>::m_object;
    };

    template<typename T, T* (*Copy)(const T*), void (*Destroy)(T*)>
    class copyable_handle : public basic_handle<T, Destroy> {
    public:
      using basic_handle<T, Destroy>::basic_handle;
      using basic_handle<T, Destroy>::destroy;

      copyable_handle(const copyable_handle& other)
      : basic_handle<T, Destroy>(Copy(other.m_object))
      {
      }

      copyable_handle& operator=(const copyable_handle& other)
      {
        if (&other == this) {
          return *this;
        }

        destroy();
        m_object = Copy(other.m_object);
        return *this;
      }

    private:
      using basic_handle<T, Destroy>::m_object;
    };

    template<typename T, void (*Destroy)(T*)>
    class non_copyable_handle : public basic_handle<T, Destroy> {
    public:
      using basic_handle<T, Destroy>::basic_handle;

      non_copyable_handle(const non_copyable_handle&) = delete;
      non_copyable_handle& operator=(const non_copyable_handle&) = delete;
    };

  }

  // utilities

  template<typename T>
  struct vec2 {
    T x;
    T y;
  };

  using vec2f = vec2<double>;
  using vec2i = vec2<int>;

  template<typename T>
  struct rect {
    T x;
    T y;
    T w;
    T h;
  };

  using rectf = rect<double>;
  using recti = rect<int>;

  struct color {
    double r;
    double g;
    double b;
    double a = 1.0;
  };

  // binding

  enum class status : std::underlying_type_t<cairo_status_t> {
    success = CAIRO_STATUS_SUCCESS,
    no_memory = CAIRO_STATUS_NO_MEMORY,
    invalid_restore = CAIRO_STATUS_INVALID_RESTORE,
    invalid_pop_group = CAIRO_STATUS_INVALID_POP_GROUP,
    no_current_point = CAIRO_STATUS_NO_CURRENT_POINT,
    invalid_matrix = CAIRO_STATUS_INVALID_MATRIX,
    invalid_status = CAIRO_STATUS_INVALID_STATUS,
    null_pointer = CAIRO_STATUS_NULL_POINTER,
    invalid_string = CAIRO_STATUS_INVALID_STRING,
    invalid_path_data = CAIRO_STATUS_INVALID_PATH_DATA,
    read_error = CAIRO_STATUS_READ_ERROR,
    write_error = CAIRO_STATUS_WRITE_ERROR,
    surface_finished = CAIRO_STATUS_SURFACE_FINISHED,
    surface_type_mismatch = CAIRO_STATUS_SURFACE_TYPE_MISMATCH,
    pattern_type_mismatch = CAIRO_STATUS_PATTERN_TYPE_MISMATCH,
    invalid_content = CAIRO_STATUS_INVALID_CONTENT,
    invalid_format = CAIRO_STATUS_INVALID_FORMAT,
    invalid_visual = CAIRO_STATUS_INVALID_VISUAL,
    file_not_found = CAIRO_STATUS_FILE_NOT_FOUND,
    invalid_dash = CAIRO_STATUS_INVALID_DASH,
    invalid_dsc_comment = CAIRO_STATUS_INVALID_DSC_COMMENT,
    invalid_index = CAIRO_STATUS_INVALID_INDEX,
    clip_not_representable = CAIRO_STATUS_CLIP_NOT_REPRESENTABLE,
    temp_file_error = CAIRO_STATUS_TEMP_FILE_ERROR,
    invalid_stride = CAIRO_STATUS_INVALID_STRIDE,
    font_type_mismatch = CAIRO_STATUS_FONT_TYPE_MISMATCH,
    user_font_immutable = CAIRO_STATUS_USER_FONT_IMMUTABLE,
    user_font_error = CAIRO_STATUS_USER_FONT_ERROR,
    negative_count = CAIRO_STATUS_NEGATIVE_COUNT,
    invalid_clusters = CAIRO_STATUS_INVALID_CLUSTERS,
    invalid_slant = CAIRO_STATUS_INVALID_SLANT,
    invalid_weight = CAIRO_STATUS_INVALID_WEIGHT,
    invalid_size = CAIRO_STATUS_INVALID_SIZE,
    user_font_not_implemented = CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED,
    device_type_mismatch = CAIRO_STATUS_DEVICE_TYPE_MISMATCH,
    device_error = CAIRO_STATUS_DEVICE_ERROR,
    invalid_mesh_construction = CAIRO_STATUS_INVALID_MESH_CONSTRUCTION,
    device_finished = CAIRO_STATUS_DEVICE_FINISHED,
    jbig2_global_missing = CAIRO_STATUS_JBIG2_GLOBAL_MISSING,
    png_error = CAIRO_STATUS_PNG_ERROR,
    freetype_error = CAIRO_STATUS_FREETYPE_ERROR,
    win32_gdi_error = CAIRO_STATUS_WIN32_GDI_ERROR,
    tag_error = CAIRO_STATUS_TAG_ERROR,
  };

  inline std::string_view to_string(status s) { return cairo_status_to_string(static_cast<cairo_status_t>(s)); }

  enum class content : std::underlying_type_t<cairo_content_t> {
    color = CAIRO_CONTENT_COLOR,
    alpha = CAIRO_CONTENT_ALPHA,
    color_alpha = CAIRO_CONTENT_COLOR_ALPHA,
  };

  enum class format : std::underlying_type_t<cairo_format_t> {
    invalid = CAIRO_FORMAT_INVALID,
    argb32 = CAIRO_FORMAT_ARGB32,
    rgb24 = CAIRO_FORMAT_RGB24,
    a8 = CAIRO_FORMAT_A8,
    a1 = CAIRO_FORMAT_A1,
    rgb16_565 = CAIRO_FORMAT_RGB16_565,
    rgb30 = CAIRO_FORMAT_RGB30,
  };

  enum class compositing_operator : std::underlying_type_t<cairo_operator_t> {
    clear = CAIRO_OPERATOR_CLEAR,

    source = CAIRO_OPERATOR_SOURCE,
    over = CAIRO_OPERATOR_OVER,
    in = CAIRO_OPERATOR_IN,
    out = CAIRO_OPERATOR_OUT,
    atop = CAIRO_OPERATOR_ATOP,

    dest = CAIRO_OPERATOR_DEST,
    dest_over = CAIRO_OPERATOR_DEST_OVER,
    dest_in = CAIRO_OPERATOR_DEST_IN,
    dest_out = CAIRO_OPERATOR_DEST_OUT,
    dest_atop = CAIRO_OPERATOR_DEST_ATOP,

    exclusive_or = CAIRO_OPERATOR_XOR,
    add = CAIRO_OPERATOR_ADD,
    saturate = CAIRO_OPERATOR_SATURATE,

    multiply = CAIRO_OPERATOR_MULTIPLY,
    screen = CAIRO_OPERATOR_SCREEN,
    overlay = CAIRO_OPERATOR_OVERLAY,
    darken = CAIRO_OPERATOR_DARKEN,
    lighten = CAIRO_OPERATOR_LIGHTEN,
    color_dodge = CAIRO_OPERATOR_COLOR_DODGE,
    color_burn = CAIRO_OPERATOR_COLOR_BURN,
    hard_light = CAIRO_OPERATOR_HARD_LIGHT,
    soft_light = CAIRO_OPERATOR_SOFT_LIGHT,
    difference = CAIRO_OPERATOR_DIFFERENCE,
    exclusion = CAIRO_OPERATOR_EXCLUSION,
    hsl_hue = CAIRO_OPERATOR_HSL_HUE,
    hsl_saturation = CAIRO_OPERATOR_HSL_SATURATION,
    hsl_color = CAIRO_OPERATOR_HSL_COLOR,
    hsl_luminosity = CAIRO_OPERATOR_HSL_LUMINOSITY,
  };

  enum class antialias : std::underlying_type_t<cairo_antialias_t> {
    preset = CAIRO_ANTIALIAS_DEFAULT,

    none = CAIRO_ANTIALIAS_NONE,
    gray = CAIRO_ANTIALIAS_GRAY,
    subpixel = CAIRO_ANTIALIAS_SUBPIXEL,

    fast = CAIRO_ANTIALIAS_FAST,
    good = CAIRO_ANTIALIAS_GOOD,
    best = CAIRO_ANTIALIAS_BEST,
  };

  enum class fill_rule : std::underlying_type_t<cairo_fill_rule_t> {
    winding = CAIRO_FILL_RULE_WINDING,
    event_odd = CAIRO_FILL_RULE_EVEN_ODD,
  };

  enum class line_cap : std::underlying_type_t<cairo_line_cap_t> {
    butt = CAIRO_LINE_CAP_BUTT,
    round = CAIRO_LINE_CAP_ROUND,
    square = CAIRO_LINE_CAP_SQUARE,
  };

  enum class line_join : std::underlying_type_t<cairo_line_join_t> {
    miter = CAIRO_LINE_JOIN_MITER,
    round = CAIRO_LINE_JOIN_ROUND,
    bevel = CAIRO_LINE_JOIN_BEVEL,
  };

  /*
   * matrix
   */

  struct matrix : private cairo_matrix_t {

    static matrix create(double  xx, double  yx, double  xy, double  yy, double  x0, double  y0) { matrix m; cairo_matrix_init(m, xx, yx, xy, yy, x0, y0); return m; }
    static matrix create_identity() { matrix m; cairo_matrix_init_identity(m); return m; }
    static matrix create_translate(double tx, double ty) { matrix m; cairo_matrix_init_translate(m, tx, ty); return m; }
    static matrix create_scale(double sx, double sy) { matrix m; cairo_matrix_init_scale(m, sx, sy); return m; }
    static matrix create_rotate(double radians) { matrix m; cairo_matrix_init_rotate(m, radians); return m; }

    void translate(double tx, double ty) { cairo_matrix_translate(this, tx, ty); }
    void scale(double sx, double sy) { cairo_matrix_scale(this, sx, sy); }
    void rotate(double radians) { cairo_matrix_rotate(this, radians); }

    status invert() { return static_cast<status>(cairo_matrix_invert(this)); }

    vec2f transform_distance(vec2f d) const { cairo_matrix_transform_distance(this, &d.x, &d.y); return d; }
    vec2f transform_point(vec2f p) const { cairo_matrix_transform_point(this, &p.x, &p.y); return p; }

    matrix operator*(const matrix& other) const { matrix res; cairo_matrix_multiply(res, this, other); return res; }

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

  enum class text_cluster_flags : std::underlying_type_t<cairo_text_cluster_flags_t> {
    none = 0,
    backward = CAIRO_TEXT_CLUSTER_FLAG_BACKWARD,
  };

  // TODO: flags operators

  using text_extents = cairo_text_extents_t;
  using font_extents = cairo_font_extents_t;

  enum class font_slant : std::underlying_type_t<cairo_font_slant_t> {
    normal = CAIRO_FONT_SLANT_NORMAL,
    italic = CAIRO_FONT_SLANT_ITALIC,
    oblique = CAIRO_FONT_SLANT_OBLIQUE,
  };

  enum class font_weight : std::underlying_type_t<cairo_font_weight_t> {
    normal = CAIRO_FONT_WEIGHT_NORMAL,
    bold = CAIRO_FONT_WEIGHT_BOLD,
  };

  enum class subpixel_order : std::underlying_type_t<cairo_subpixel_order_t> {
    preset = CAIRO_SUBPIXEL_ORDER_DEFAULT,
    rgb = CAIRO_SUBPIXEL_ORDER_RGB,
    bgr = CAIRO_SUBPIXEL_ORDER_BGR,
    vrgb = CAIRO_SUBPIXEL_ORDER_VRGB,
    vbgr = CAIRO_SUBPIXEL_ORDER_VBGR,
  };

  enum class hint_style : std::underlying_type_t<cairo_hint_style_t> {
    preset = CAIRO_HINT_STYLE_DEFAULT,
    none = CAIRO_HINT_STYLE_NONE,
    slight = CAIRO_HINT_STYLE_SLIGHT,
    medium = CAIRO_HINT_STYLE_MEDIUM,
    full = CAIRO_HINT_STYLE_FULL,
  };

  enum class hint_metrics : std::underlying_type_t<cairo_hint_metrics_t> {
    preset = CAIRO_HINT_METRICS_DEFAULT,
    off = CAIRO_HINT_METRICS_OFF,
    on = CAIRO_HINT_METRICS_ON,
  };

  class font_options {
  public:
    font_options()
    : m_options(cairo_font_options_create())
    {
    }

    status get_status() { return static_cast<status>(cairo_font_options_status(m_options)); }
    void merge(const font_options& other) { cairo_font_options_merge(m_options, other.m_options); }
    unsigned long hash() const { return cairo_font_options_hash(m_options); }

    void set_antialias(antialias aa) { cairo_font_options_set_antialias(m_options, static_cast<cairo_antialias_t>(aa)); }
    antialias get_antialias() const { return static_cast<antialias>(cairo_font_options_get_antialias(m_options)); }

    void set_subpixel_order(subpixel_order so) { cairo_font_options_set_subpixel_order(m_options, static_cast<cairo_subpixel_order_t>(so)); }
    subpixel_order get_subpixel_order() const { return static_cast<subpixel_order>(cairo_font_options_get_subpixel_order(m_options)); }

    void set_hint_style(hint_style hs) { cairo_font_options_set_hint_style(m_options, static_cast<cairo_hint_style_t>(hs)); }
    hint_style get_hint_style() const { return static_cast<hint_style>(cairo_font_options_get_hint_style(m_options)); }

    void set_hint_metrics(hint_metrics hm) { cairo_font_options_set_hint_metrics(m_options, static_cast<cairo_hint_metrics_t>(hm)); }
    hint_metrics get_hint_metrics() const { return static_cast<hint_metrics>(cairo_font_options_get_hint_metrics(m_options)); }

    void set_variations(const char* variations) { cairo_font_options_set_variations(m_options, variations); }
    const char* get_variations() { return cairo_font_options_get_variations(m_options); }

    bool operator==(const font_options& other) const { return cairo_font_options_equal(m_options, other.m_options); }

  private:
    friend class context;
    friend class scaled_font;
    friend class surface;
    details::copyable_handle<cairo_font_options_t, cairo_font_options_copy, cairo_font_options_destroy> m_options;
  };

  enum class font_type {
    toy = CAIRO_FONT_TYPE_TOY,
    ft = CAIRO_FONT_TYPE_FT,
    win32 = CAIRO_FONT_TYPE_WIN32,
    quartz = CAIRO_FONT_TYPE_QUARTZ,
    user = CAIRO_FONT_TYPE_USER
  };

  class toy_font_face;

  class font_face {
  public:

    status get_status() { return static_cast<status>(cairo_font_face_status(m_font)); }
    font_type get_type() { return static_cast<font_type>(cairo_font_face_get_type(m_font)); }

    inline toy_font_face& as_toy();

  protected:
    font_face(cairo_font_face_t* font, details::increase_reference_type)
    : m_font(font, details::increase_reference)
    {
    }

    struct derived_font_type {};
    static constexpr derived_font_type derived_font = {};

    font_face(cairo_font_face_t* font, derived_font_type)
    : m_font(font)
    {
    }

    friend class context;
    friend class scaled_font;
    details::handle<cairo_font_face_t, cairo_font_face_reference, cairo_font_face_destroy> m_font;
  };

  struct text_glyphs {
    status result;
    std::vector<glyph> glyphs;
    std::vector<text_cluster> clusters;
    text_cluster_flags flags;
  };

  class toy_font_face : public font_face {
  public:
    toy_font_face(const char* family, font_slant slant, font_weight weight)
    : font_face(cairo_toy_font_face_create(family, static_cast<cairo_font_slant_t>(slant), static_cast<cairo_font_weight_t>(weight)), derived_font)
    {
    }

    const char* get_family() { return cairo_toy_font_face_get_family(m_font); }
    font_slant get_slant() { return static_cast<font_slant>(cairo_toy_font_face_get_slant(m_font)); }
    font_weight get_weight() { return static_cast<font_weight>(cairo_toy_font_face_get_weight(m_font)); }
  };

  inline toy_font_face& font_face::as_toy() {
    assert(get_type() == font_type::toy);
    return static_cast<toy_font_face&>(*this);
  }


  class scaled_font {
  public:
    scaled_font(font_face& font, const matrix& font_matrix, const matrix& ctm, const font_options& options)
    : m_font(cairo_scaled_font_create(font.m_font, font_matrix, ctm, options.m_options))
    {
    }

    status get_status() { return static_cast<status>(cairo_scaled_font_status(m_font)); }
    font_type get_type() { return static_cast<font_type>(cairo_scaled_font_get_type(m_font)); }

    font_extents get_font_extents() { font_extents extents; cairo_scaled_font_extents(m_font, &extents); return extents; }
    text_extents get_text_extents(const char* utf8) { text_extents extents; cairo_scaled_font_text_extents(m_font, utf8, &extents); return extents; }
    text_extents get_glyph_extents(const glyph* glyphs, int num_glyphs) { text_extents extents; cairo_scaled_font_glyph_extents(m_font, glyphs, num_glyphs, &extents); return extents; }
    template<typename T>
    text_extents get_glyph_extents(const T& glyphs) { text_extents extents; cairo_scaled_font_glyph_extents(m_font, std::data(glyphs), static_cast<int>(std::size(glyphs)), &extents); return extents; }

    text_glyphs text_to_glyphs(double x, double y, const char* utf8, int utf8_len)
    {
      glyph* glyphs = nullptr;
      int num_glyphs;
      text_cluster* clusters = nullptr;
      int num_clusters;
      cairo_text_cluster_flags_t flags;

      text_glyphs ret;

      ret.result = static_cast<enum status>(cairo_scaled_font_text_to_glyphs(m_font, x, y, utf8, utf8_len, &glyphs, &num_glyphs, &clusters, &num_clusters, &flags));

      if (ret.result == status::success) {
        ret.glyphs.assign(glyphs, glyphs + num_glyphs);
        cairo_glyph_free(glyphs);
        ret.clusters.assign(clusters, clusters + num_clusters);
        cairo_text_cluster_free(clusters);
        ret.flags = static_cast<text_cluster_flags>(flags);
      }

      return ret;
    }

    font_face get_font_face() { return { cairo_scaled_font_get_font_face(m_font), details::increase_reference }; }
    matrix get_font_matrix() { matrix m; cairo_scaled_font_get_font_matrix(m_font, m); return m; }
    matrix get_ctm() { matrix m; cairo_scaled_font_get_ctm(m_font, m); return m; }
    matrix get_scale_matrix() { matrix m; cairo_scaled_font_get_scale_matrix(m_font, m); return m; }
    font_options get_font_options() { font_options opt; cairo_scaled_font_get_font_options(m_font, opt.m_options); return opt; }

  private:
    scaled_font(cairo_scaled_font_t* font, details::increase_reference_type)
    : m_font(font, details::increase_reference)
    {
    }

    friend class context;
    details::handle<cairo_scaled_font_t, cairo_scaled_font_reference, cairo_scaled_font_destroy> m_font;
  };

  /*
   * path
   */

  enum class path_data_type : std::underlying_type_t<cairo_path_data_type_t> {
    move_to = CAIRO_PATH_MOVE_TO,
    line_to = CAIRO_PATH_LINE_TO,
    curve_to = CAIRO_PATH_CURVE_TO,
    close_path = CAIRO_PATH_CLOSE_PATH,
  };

  using path_data = cairo_path_data_t;

  class path_element {
  public:
    path_data_type get_type() const { return static_cast<path_data_type>(m_data[0].header.type);  }
    int length() const { return m_data[0].header.length; }

    vec2f get_point(int i) const
    {
      assert(i < length());
      return { m_data[i].point.x, m_data[i].point.y };
    }

    vec2f operator[](int i) const { return get_point(i); }

    constexpr bool operator==(const path_element& other) const noexcept { return m_data == other.m_data; }
    constexpr bool operator!=(const path_element& other) const noexcept { return m_data != other.m_data; }

  private:
    path_element(const path_data* data)
    : m_data(data)
    {
    }

    friend class path_iterator;
    const path_data* m_data = nullptr;
  };

  struct path_sentinel {
  };

  class path_iterator {
  public:
    using value_type = path_element;
    using difference_type = int;
    using reference = value_type;
    using pointer = value_type;
    using iterator_category = std::forward_iterator_tag;

    reference operator*() { return m_data; }
    pointer operator->() { return m_data; }

    path_iterator& operator++() { m_data += m_data[0].header.length; return *this; }
    path_iterator operator++(int) { path_iterator copy = *this; ++copy; return copy; }

    constexpr bool operator==(const path_iterator& other) const noexcept { return m_data == other.m_data; }
    constexpr bool operator!=(const path_iterator& other) const noexcept { return m_data != other.m_data; }

  private:
    friend class path;
    path_iterator(const path_data* data)
    : m_data(data)
    {
    }

    const path_data* m_data = nullptr;
  };

  class path {
  public:
    status get_status() const { return static_cast<status>(m_path.get()->status); };

    path_iterator begin() const { return m_path.get()->data; }
    path_iterator end() const { return m_path.get()->data + m_path.get()->num_data; }

  private:
    path(cairo_path* p)
    : m_path(p)
    {
    }

    friend class context;
    friend class mesh_pattern;
    details::non_copyable_handle<cairo_path, cairo_path_destroy> m_path;
  };

  /*
   * pattern
   */

  enum class pattern_type : std::underlying_type_t<cairo_pattern_type_t> {
    solid = CAIRO_PATTERN_TYPE_SOLID,
    surface = CAIRO_PATTERN_TYPE_SURFACE,
    linear = CAIRO_PATTERN_TYPE_LINEAR,
    radial = CAIRO_PATTERN_TYPE_RADIAL,
    mesh = CAIRO_PATTERN_TYPE_MESH,
    raster_source = CAIRO_PATTERN_TYPE_RASTER_SOURCE,
  };

  enum class extend : std::underlying_type_t<cairo_extend_t> {
    none = CAIRO_EXTEND_NONE,
    repeat = CAIRO_EXTEND_REPEAT,
    reflect = CAIRO_EXTEND_REFLECT,
    pad = CAIRO_EXTEND_PAD,
  };

  enum class filter : std::underlying_type_t<cairo_filter_t> {
    fast = CAIRO_FILTER_FAST,
    good = CAIRO_FILTER_GOOD,
    best = CAIRO_FILTER_BEST,
    nearest = CAIRO_FILTER_NEAREST,
    bilinear = CAIRO_FILTER_BILINEAR,
    gaussian = CAIRO_FILTER_GAUSSIAN,
  };

  class pattern {
  public:
    status get_status() { return static_cast<status>(cairo_pattern_status(m_pattern)); }
    pattern_type get_type() { return static_cast<pattern_type>(cairo_pattern_get_type(m_pattern)); }

    void set_matrix(const matrix& m) { cairo_pattern_set_matrix(m_pattern, m); }
    matrix get_matrix() { matrix m; cairo_pattern_get_matrix(m_pattern, m); return m; }

    void set_extend(extend e) { cairo_pattern_set_extend(m_pattern, static_cast<cairo_extend_t>(e)); }
    extend get_extend() { return static_cast<extend>(cairo_pattern_get_extend(m_pattern)); }

    void set_filter(filter f) { cairo_pattern_set_filter(m_pattern, static_cast<cairo_filter_t>(f)); }
    filter get_filter() { return static_cast<filter>(cairo_pattern_get_filter(m_pattern)); }

  protected:
    pattern(cairo_pattern_t* pat)
    : m_pattern(pat)
    {
    }

    pattern(cairo_pattern_t* pat, details::increase_reference_type)
    : m_pattern(pat, details::increase_reference)
    {
    }

    friend class context;
    details::handle<cairo_pattern_t, cairo_pattern_reference, cairo_pattern_destroy> m_pattern;
  };

  /*
   * device
   */

  enum class device_type : std::underlying_type_t<cairo_device_type_t> {
    drm = CAIRO_DEVICE_TYPE_DRM,
    gl = CAIRO_DEVICE_TYPE_GL,
    script = CAIRO_DEVICE_TYPE_SCRIPT,
    xcb = CAIRO_DEVICE_TYPE_XCB,
    xlib = CAIRO_DEVICE_TYPE_XLIB,
    xml = CAIRO_DEVICE_TYPE_XML,
    cogl = CAIRO_DEVICE_TYPE_COGL,
    win32 = CAIRO_DEVICE_TYPE_WIN32,

    invalid = CAIRO_DEVICE_TYPE_INVALID,
  };

  class device {
  public:
    device_type get_type() { return static_cast<device_type>(cairo_device_get_type(m_device)); }
    status get_status() { return static_cast<status>(cairo_device_status(m_device)); }

    status acquire() { return static_cast<status>(cairo_device_acquire(m_device)); }
    void release() { cairo_device_release(m_device); }

    void flush() { cairo_device_flush(m_device); }
    void finish() { cairo_device_finish(m_device); }

  private:
    device(cairo_device_t* dev, details::increase_reference_type)
    : m_device(dev, details::increase_reference)
    {
    }

    friend class surface;
    details::handle<cairo_device_t, cairo_device_reference, cairo_device_destroy> m_device;
  };

  /*
   * surface
   */

  enum class surface_type : std::underlying_type_t<cairo_surface_type_t> {
    image = CAIRO_SURFACE_TYPE_IMAGE,
    pdf = CAIRO_SURFACE_TYPE_PDF,
    ps = CAIRO_SURFACE_TYPE_PS,
    xlib = CAIRO_SURFACE_TYPE_XLIB,
    xcb = CAIRO_SURFACE_TYPE_XCB,
    glitz = CAIRO_SURFACE_TYPE_GLITZ,
    quartz = CAIRO_SURFACE_TYPE_QUARTZ,
    win32 = CAIRO_SURFACE_TYPE_WIN32,
    beos = CAIRO_SURFACE_TYPE_BEOS,
    directfb = CAIRO_SURFACE_TYPE_DIRECTFB,
    svg = CAIRO_SURFACE_TYPE_SVG,
    os2 = CAIRO_SURFACE_TYPE_OS2,
    win32_printing = CAIRO_SURFACE_TYPE_WIN32_PRINTING,
    quartz_image = CAIRO_SURFACE_TYPE_QUARTZ_IMAGE,
    script = CAIRO_SURFACE_TYPE_SCRIPT,
    qt = CAIRO_SURFACE_TYPE_QT,
    recording = CAIRO_SURFACE_TYPE_RECORDING,
    vg = CAIRO_SURFACE_TYPE_VG,
    gl = CAIRO_SURFACE_TYPE_GL,
    drm = CAIRO_SURFACE_TYPE_DRM,
    tee = CAIRO_SURFACE_TYPE_TEE,
    xml = CAIRO_SURFACE_TYPE_XML,
    skia = CAIRO_SURFACE_TYPE_SKIA,
    subsurface = CAIRO_SURFACE_TYPE_SUBSURFACE,
    cogl = CAIRO_SURFACE_TYPE_COGL,
  };

  class surface {
  public:
    status get_status() { return static_cast<status>(cairo_surface_status(m_surface)); }
    surface_type get_type() { return static_cast<surface_type>(cairo_surface_get_type(m_surface)); }
    content get_content() { return static_cast<content>(cairo_surface_get_content(m_surface)); }

    surface create_similar(content cnt, int width, int height) { return cairo_surface_create_similar(m_surface, static_cast<cairo_content_t>(cnt), width, height); }
    surface create_similar(content cnt, vec2i size) { return cairo_surface_create_similar(m_surface, static_cast<cairo_content_t>(cnt), size.x, size.y); }
    surface create_similar_image(format fmt, int width, int height) { return cairo_surface_create_similar_image(m_surface, static_cast<cairo_format_t>(fmt), width, height); }
    surface create_similar_image(format fmt, vec2i size) { return cairo_surface_create_similar_image(m_surface, static_cast<cairo_format_t>(fmt), size.x, size.y); }
    surface create_for_rectangle(double x, double y, double width, double height) { return cairo_surface_create_for_rectangle(m_surface, x, y, width, height); }
    surface create_for_rectangle(rectf rectangle) { return cairo_surface_create_for_rectangle(m_surface, rectangle.x, rectangle.y, rectangle.w, rectangle.h); }

    void finish() { cairo_surface_finish(m_surface); }

    device get_device() { return { cairo_surface_get_device(m_surface), details::increase_reference };  }

#if CAIRO_HAS_PNG_FUNCTIONS
    status write_to_png(const char* filename) { return static_cast<status>(cairo_surface_write_to_png(m_surface, filename)); }
    status write_to_png(const std::filesystem::path& filename) { return static_cast<status>(cairo_surface_write_to_png(m_surface, filename.string().c_str())); }
#endif

    font_options get_font_options() { font_options opt; cairo_surface_get_font_options(m_surface, opt.m_options); return opt; };

    void flush() { cairo_surface_flush(m_surface); }
    void mark_dirty() { cairo_surface_mark_dirty(m_surface); }
    void mark_dirty_rectangle(int x, int y, int width, int height) { cairo_surface_mark_dirty_rectangle(m_surface, x, y, width, height); }
    void mark_dirty_rectangle(recti rectangle) { cairo_surface_mark_dirty_rectangle(m_surface, rectangle.x, rectangle.y, rectangle.w, rectangle.h); }

    void set_device_scale(double x_scale, double y_scale) { cairo_surface_set_device_scale(m_surface, x_scale, y_scale); }
    void set_device_scale(vec2f scale) { cairo_surface_set_device_scale(m_surface, scale.x, scale.y); }
    vec2f get_device_scale() { vec2f scale; cairo_surface_get_device_scale(m_surface, &scale.x, &scale.y); return scale; }
    void set_device_offset(double x_offset, double y_offset) { cairo_surface_set_device_offset(m_surface, x_offset, y_offset); }
    void set_device_offset(vec2f offset) { cairo_surface_set_device_offset(m_surface, offset.x, offset.y); }
    vec2f get_device_offset() { vec2f offset; cairo_surface_get_device_offset(m_surface, &offset.x, &offset.y); return offset; }
    void set_fallback_resolution(double x_pixels_per_inch, double y_pixels_per_inch) { cairo_surface_set_fallback_resolution(m_surface, x_pixels_per_inch, y_pixels_per_inch); }
    void set_fallback_resolution(vec2f pixels_per_inch) { cairo_surface_set_fallback_resolution(m_surface, pixels_per_inch.x, pixels_per_inch.y); }
    vec2f get_fallback_resolution() { vec2f pixels_per_inch; cairo_surface_get_fallback_resolution(m_surface, &pixels_per_inch.x, &pixels_per_inch.y); return pixels_per_inch; }

    void copy_page() { cairo_surface_copy_page(m_surface); }
    void show_page() { cairo_surface_show_page(m_surface); }
    bool has_show_text_glyphs() { return cairo_surface_has_show_text_glyphs(m_surface); }

  protected:
    surface(cairo_surface_t* surf)
    : m_surface(surf)
    {
    }

    surface(cairo_surface_t* surf, details::increase_reference_type)
    : m_surface(surf, details::increase_reference)
    {
    }

    friend class context;
    friend class surface_pattern;
    details::handle<cairo_surface_t, cairo_surface_reference, cairo_surface_destroy> m_surface;
  };

  /*
   * context
   */

  struct dash_array {
    std::vector<double> dashes;
    double offset;
  };

  class context {
  public:
    context(surface& surf)
    : m_context(cairo_create(surf.m_surface))
    {
    }

    status get_status() { return static_cast<status>(cairo_status(m_context)); }
    surface get_target() { return { cairo_get_target(m_context), details::increase_reference }; }

    void save() { cairo_save(m_context); }
    void restore() { cairo_restore(m_context); }

    void push_group() { cairo_push_group(m_context); }
    void push_group_with_content(content c) { cairo_push_group_with_content(m_context, static_cast<cairo_content_t>(c)); }
    pattern pop_group() { return pattern(cairo_pop_group(m_context)); }
    void pop_group_to_source() { cairo_pop_group_to_source(m_context); }
    surface get_group_target() { return { cairo_get_group_target(m_context), details::increase_reference }; }

    // modify state

    void set_operator(compositing_operator op) { cairo_set_operator(m_context, static_cast<cairo_operator_t>(op)); }
    compositing_operator get_operator() { return static_cast<compositing_operator>(cairo_get_operator(m_context)); }
    void set_source(pattern& pat) { cairo_set_source(m_context, pat.m_pattern); }
    pattern get_source() { return { cairo_get_source(m_context), details::increase_reference }; }
    void set_source(surface& surf, double x, double y) { cairo_set_source_surface(m_context, surf.m_surface, x, y); }
    void set_source(surface& surf, vec2f origin) { cairo_set_source_surface(m_context, surf.m_surface, origin.x, origin.y); }
    void set_source_rgb(double red, double green, double blue) { cairo_set_source_rgb(m_context, red, green, blue); }
    void set_source_rgba(double red, double green, double blue, double alpha) { cairo_set_source_rgba(m_context, red, green, blue, alpha); }
    void set_source_color(color col) { cairo_set_source_rgba(m_context, col.r, col.g, col.b, col.a); }
    void set_tolerance(double tolerance) { cairo_set_tolerance(m_context, tolerance); }
    double get_tolerence() { return cairo_get_tolerance(m_context); }

    void set_antialias(antialias aa) { cairo_set_antialias(m_context, static_cast<cairo_antialias_t>(aa)); }
    antialias get_antialias() { return static_cast<antialias>(cairo_get_antialias(m_context)); }

    void set_fill_rule(fill_rule fr) { cairo_set_fill_rule(m_context, static_cast<cairo_fill_rule_t>(fr)); }
    fill_rule get_fill_rule() { return static_cast<fill_rule>(cairo_get_fill_rule(m_context)); }
    void set_line_width(double width) { cairo_set_line_width(m_context, width); }
    double get_line_width() { return cairo_get_line_width(m_context); }
    void set_line_cap(line_cap lc) { cairo_set_line_cap(m_context, static_cast<cairo_line_cap_t>(lc)); }
    line_cap get_line_cap() { return static_cast<line_cap>(cairo_get_line_cap(m_context)); }
    void set_line_join(line_join lj) { cairo_set_line_join(m_context, static_cast<cairo_line_join_t>(lj)); }
    line_join get_line_join() { return static_cast<line_join>(cairo_get_line_join(m_context)); }

    void set_dash(const double* dashes, int num_dashes, double offset) { cairo_set_dash(m_context, dashes, num_dashes, offset); };
    template<typename T>
    void set_dash(const T& dashes, double offset) { cairo_set_dash(m_context, std::data(dashes), static_cast<int>(std::size(dashes)), offset); }
    int get_dash_count() { return cairo_get_dash_count(m_context); }
    dash_array get_dash()
    {
      const int count = get_dash_count();
      dash_array array;
      array.dashes.resize(count);
      cairo_get_dash(m_context, array.dashes.data(), &array.offset);
      return array;
    }

    void set_miter_limit(double limit) { cairo_set_miter_limit(m_context, limit); }
    double get_mitter_limit() { return cairo_get_miter_limit(m_context); }

    context& translate(double tx, double ty) { cairo_translate(m_context, tx, ty); return *this; };
    context& translate(vec2f translation) { cairo_translate(m_context, translation.x, translation.y); return *this; };
    context& scale(double sx, double sy) { cairo_scale(m_context, sx, sy); return *this; }
    context& scale(vec2f scale) { cairo_scale(m_context, scale.x, scale.y); return *this; }
    context& rotate(double angle) { cairo_rotate(m_context, angle); return *this; }
    context& transform(const matrix& m) { cairo_transform(m_context, m); return *this; }

    void set_matrix(const matrix& m) { cairo_set_matrix(m_context, m); }
    matrix get_matrix() { matrix m; cairo_get_matrix(m_context, m); return m; }
    void identity_matrix() { cairo_identity_matrix(m_context); }

    vec2f user_to_device(double x, double y) { cairo_user_to_device(m_context, &x, &y); return { x, y }; }
    vec2f user_to_device(vec2f point) { cairo_user_to_device(m_context, &point.x, &point.y); return point; }
    vec2f user_to_device_distance(double dx, double dy) { cairo_user_to_device_distance(m_context, &dx, &dy); return { dx, dy }; }
    vec2f user_to_device_distance(vec2f distance) { cairo_user_to_device_distance(m_context, &distance.x, &distance.y); return distance; }
    vec2f device_to_user(double x, double y) { cairo_device_to_user(m_context, &x, &y); return { x, y }; }
    vec2f device_to_user(vec2f point) { cairo_device_to_user(m_context, &point.x, &point.y); return point; }
    vec2f device_to_user_distance(double dx, double dy) { cairo_device_to_user_distance(m_context, &dx, &dy); return { dx, dy }; }
    vec2f device_to_user_distance(vec2f distance) { cairo_device_to_user_distance(m_context, &distance.x, &distance.y); return distance; }

    // path creation function

    context& new_path() { cairo_new_path(m_context); return *this; }
    context& new_sub_path() { cairo_new_sub_path(m_context); return *this; }
    context& move_to(double x, double y) { cairo_move_to(m_context, x, y); return *this; }
    context& move_to(vec2f point) { cairo_move_to(m_context, point.x, point.y); return *this; }
    context& line_to(double x, double y) { cairo_line_to(m_context, x, y); return *this; }
    context& line_to(vec2f point) { cairo_line_to(m_context, point.x, point.y); return *this; }
    context& curve_to(double x1, double y1, double x2, double y2, double x3, double y3) { cairo_curve_to(m_context, x1, y1, x2, y2, x3, y3); return *this; }
    context& curve_to(vec2f p1, vec2f p2, vec2f p3) { cairo_curve_to(m_context, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y); return *this; }
    context& arc(double xc, double yc, double radius, double angle1, double angle2) { cairo_arc(m_context, xc, yc, radius, angle1, angle2); return *this; }
    context& arc(vec2f center, double radius, double angle1, double angle2) { cairo_arc(m_context, center.x, center.y, radius, angle1, angle2); return *this; }
    context& arc_negative(double xc, double yc, double radius, double angle1, double angle2) { cairo_arc_negative(m_context, xc, yc, radius, angle1, angle2); return *this; }
    context& arc_negative(vec2f center, double radius, double angle1, double angle2) { cairo_arc_negative(m_context, center.x, center.y, radius, angle1, angle2); return *this; }
    context& rel_move_to(double dx, double dy) { cairo_rel_move_to(m_context, dx, dy); return *this; }
    context& rel_move_to(vec2f d) { cairo_rel_move_to(m_context, d.x, d.y); return *this; }
    context& rel_line_to(double dx, double dy) { cairo_rel_line_to(m_context, dx, dy); return *this; }
    context& rel_line_to(vec2f d) { cairo_rel_line_to(m_context, d.x, d.y); return *this; }
    context& rel_curve_to(double dx1, double dy1, double dx2, double dy2, double dx3, double dy3) { cairo_rel_curve_to(m_context, dx1, dy1, dx2, dy2, dx3, dy3); return *this; }
    context& rel_curve_to(vec2f d1, vec2f d2, vec2f d3) { cairo_curve_to(m_context, d1.x, d1.y, d2.x, d2.y, d3.x, d3.y); return *this; }
    context& rectangle(double x, double y, double w, double h) { cairo_rectangle(m_context, x, y, w, h); return *this; }
    context& rectangle(const rectf& r) { cairo_rectangle(m_context, r.x, r.y, r.w, r.h); return *this; }
    void close_path() { cairo_close_path(m_context); }

    rectf path_extents() { double x1, y1, x2, y2; cairo_path_extents(m_context, &x1, &y1, &x2, &y2); return { x1, y1, x2 - x1, y2 -y1 }; }

    bool has_current_point() { return cairo_has_current_point(m_context); }
    vec2f get_current_point() { vec2f point; cairo_get_current_point(m_context, &point.x, &point.y); return point; }

    path copy_path() { return cairo_copy_path(m_context); }
    path copy_path_flat() { return cairo_copy_path_flat(m_context); }
    void append_path(const path& p) { cairo_append_path(m_context, p.m_path); }

    // painting

    void paint() { cairo_paint(m_context); }
    void paint_with_alpha(double alpha) { cairo_paint_with_alpha(m_context, alpha); }
    void mask(pattern& pat) { cairo_mask(m_context, pat.m_pattern); }
    void mask(surface& surf, double surface_x, double surface_y) { cairo_mask_surface(m_context, surf.m_surface, surface_x, surface_y); }
    void mask(surface& surf, vec2f origin) { cairo_mask_surface(m_context, surf.m_surface, origin.x, origin.y); }
    void stroke() { cairo_stroke(m_context); }
    void stroke_preserve() { cairo_stroke_preserve(m_context); }
    void fill() { cairo_fill(m_context); }
    void fill_preserve() { cairo_fill_preserve(m_context); }
    void copy_page() { cairo_copy_page(m_context); }
    void show_page() { cairo_show_page(m_context); }

    // insideness testing

    bool in_stroke(double x, double y) { return cairo_in_stroke(m_context, x, y); }
    bool in_stroke(vec2f point) { return cairo_in_stroke(m_context, point.x, point.y); }
    bool in_fill(double x, double y) { return cairo_in_fill(m_context, x, y); }
    bool in_fill(vec2f point) { return cairo_in_fill(m_context, point.x, point.y); }
    bool in_clip(double x, double y) { return cairo_in_clip(m_context, x, y); }
    bool in_clip(vec2f point) { return cairo_in_clip(m_context, point.x, point.y); }

    // rectangular extents

    rectf stroke_extents() { double x1, y1, x2, y2; cairo_stroke_extents(m_context, &x1, &y1, &x2, &y2); return { x1, y1, x2 - x1, y2 -y1 }; }
    rectf fill_extents() { double x1, y1, x2, y2; cairo_fill_extents(m_context, &x1, &y1, &x2, &y2); return { x1, y1, x2 - x1, y2 -y1 }; }

    // clipping

    void reset_clip() { cairo_reset_clip(m_context); }
    void clip() { cairo_clip(m_context); }
    void clip_preserve() { cairo_clip_preserve(m_context); }
    rectf clip_extents() { double x1, y1, x2, y2; cairo_clip_extents(m_context, &x1, &y1, &x2, &y2); return { x1, y1, x2 - x1, y2 -y1 }; }
    std::vector<rectf> clip_rectangle_list()
    {
      std::unique_ptr<cairo_rectangle_list_t, decltype(&cairo_rectangle_list_destroy)> ptr(cairo_copy_clip_rectangle_list(m_context), cairo_rectangle_list_destroy);

      if (ptr->status != CAIRO_STATUS_SUCCESS) {
        return {};
      }

      std::vector<rectf> list;

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

    void select_font_face(const char* family, font_slant slant, font_weight weight) { cairo_select_font_face(m_context, family, static_cast<cairo_font_slant_t>(slant), static_cast<cairo_font_weight_t>(weight)); }
    void set_font_size(double size) { cairo_set_font_size(m_context, size); }
    void set_font_matrix(const matrix& m) { cairo_set_font_matrix(m_context, m); }
    matrix get_font_matrix() { matrix m; cairo_get_font_matrix(m_context, m); return m; }
    void set_font_options(const font_options& opt) { cairo_set_font_options(m_context, opt.m_options); }
    font_options get_font_options() { font_options opt; cairo_get_font_options(m_context, opt.m_options); return opt; }
    void set_font_face(font_face& font) { cairo_set_font_face(m_context, font.m_font); }
    font_face get_font_face() { return { cairo_get_font_face(m_context), details::increase_reference }; }
    void set_scaled_font(scaled_font& font) { cairo_set_scaled_font(m_context, font.m_font); }
    scaled_font get_scaled_font() { return { cairo_get_scaled_font(m_context), details::increase_reference }; }

    void show_text(const char* utf8) { cairo_show_text(m_context, utf8); }
    void show_glyphs(const glyph* glyphs, int num_glyphs) { cairo_show_glyphs(m_context, glyphs, num_glyphs); }
    template<typename T>
    void show_glyphs(const T& glyphs) { cairo_show_glyphs(m_context, std::data(glyphs), static_cast<int>(std::size(glyphs))); }
    void show_text_glyphs(const char* utf8, int utf8_len, const glyph* glyphs, int num_glyphs, const text_cluster* clusters, int num_clusters, text_cluster_flags cluster_flags = text_cluster_flags::none) { cairo_show_text_glyphs(m_context, utf8, utf8_len, glyphs, num_glyphs, clusters, num_clusters, static_cast<cairo_text_cluster_flags_t>(cluster_flags)); }
    template<typename T, typename U>
    void show_text_glyphs(std::string_view utf8, const T& glyphs, const U& clusters, text_cluster_flags cluster_flags = text_cluster_flags::none) { cairo_show_text_glyphs(m_context, std::data(utf8), static_cast<int>(std::size(utf8)), std::data(glyphs), static_cast<int>(std::size(glyphs)), std::data(clusters), static_cast<int>(std::size(clusters)), cluster_flags); }
    void text_path(const char* utf8) { cairo_text_path(m_context, utf8); }
    void glyph_path(const glyph* glyphs, int num_glyphs) { cairo_glyph_path(m_context, glyphs, num_glyphs); }
    template<typename T>
    void glyph_path(const T& glyphs) { cairo_glyph_path(m_context, std::data(glyphs), static_cast<int>(std::size(glyphs))); }

    text_extents get_text_extents(const char* utf8) { text_extents extents; cairo_text_extents(m_context, utf8, &extents); return extents; }
    text_extents get_glyph_extents(const glyph* glyphs, int num_glyphs) { text_extents extents; cairo_glyph_extents(m_context, glyphs, num_glyphs, &extents); return extents; }
    template<typename T>
    text_extents get_glyph_extents(const T& glyphs) { text_extents extents; cairo_glyph_extents(m_context, std::data(glyphs), static_cast<int>(std::size(glyphs)), &extents); return extents; }
    font_extents get_font_extents() { font_extents extents; cairo_font_extents(m_context, &extents); return extents; }

  private:
    details::handle<cairo_t, cairo_reference, cairo_destroy> m_context;
  };

  class subcontext {
  public:
    subcontext(context& ctx)
    : m_context(&ctx)
    {
      m_context->save();
    }

    subcontext(const subcontext&) = delete;
    subcontext(subcontext&&) noexcept = delete;

    ~subcontext()
    {
      m_context->restore();
    }

    subcontext& operator=(const subcontext&) = delete;
    subcontext& operator=(subcontext&&) noexcept = delete;

  private:
    context* m_context = nullptr;
  };

  /*
   * patterns
   */

  class solid_pattern : public pattern {
  public:
    static solid_pattern create_rgb(double red, double green, double blue) { return cairo_pattern_create_rgb(red, green, blue); }
    static solid_pattern create_rgba(double red, double green, double blue, double alpha) { return cairo_pattern_create_rgba(red, green, blue, alpha); }
    static solid_pattern create_color(color col) { return cairo_pattern_create_rgba(col.r, col.g, col.b, col.a); }

    color get_color() { color col; cairo_pattern_get_rgba(m_pattern, &col.r, &col.g, &col.b, &col.a); return col; }


  private:
    solid_pattern(cairo_pattern_t* pat)
    : pattern(pat)
    {
    }
  };

  class surface_pattern : public pattern {
  public:
    static surface_pattern create(surface& surf) { return cairo_pattern_create_for_surface(surf.m_surface); }

    surface get_surface() { cairo_surface_t* surface; cairo_pattern_get_surface(m_pattern, &surface); return { surface, details::increase_reference }; }

  private:
    surface_pattern(cairo_pattern_t* pat)
    : pattern(pat)
    {
    }
  };

  class gradient_pattern : public pattern {
  public:

    void add_color_stop_rgb(double offset, double red, double green, double blue) { cairo_pattern_add_color_stop_rgb(m_pattern, offset, red, green, blue); }
    void add_color_stop_rgba(double offset, double red, double green, double blue, double alpha) { cairo_pattern_add_color_stop_rgba(m_pattern, offset, red, green, blue, alpha); }
    void add_color_stop_color(double offset, color col) { cairo_pattern_add_color_stop_rgba(m_pattern, offset, col.r, col.g, col.b, col.a); }

    std::pair<double, color> get_color_stop(int index)
    {
      double offset;
      color col;
      [[maybe_unused]] auto result = cairo_pattern_get_color_stop_rgba(m_pattern, index, &offset, &col.r, &col.g, &col.b, &col.a);
      assert(result == CAIRO_STATUS_SUCCESS);
      return { offset, col };
    }

    int get_color_stop_count()
    {
      int count;
      [[maybe_unused]] auto result = cairo_pattern_get_color_stop_count(m_pattern, &count);
      assert(result == CAIRO_STATUS_SUCCESS);
      return count;
    }

  protected:
    gradient_pattern(cairo_pattern_t* pat)
    : pattern(pat)
    {
    }
  };

  class linear_gradient_pattern : public gradient_pattern {
  public:
    static linear_gradient_pattern create(double x0, double y0, double x1, double y1) { return cairo_pattern_create_linear(x0, y0, x1, y1); }
    static linear_gradient_pattern create(vec2f p0, vec2f p1) { return cairo_pattern_create_linear(p0.x, p0.y, p1.x, p1.y); }

    std::pair<vec2f, vec2f> get_linear_points()
    {
      vec2f p0, p1;
      [[maybe_unused]] auto result = cairo_pattern_get_linear_points(m_pattern, &p0.x, &p0.y, &p1.x, &p1.y);
      assert(result == CAIRO_STATUS_SUCCESS);
      return { p0, p1 };
    }

  private:
    linear_gradient_pattern(cairo_pattern_t* pat)
    : gradient_pattern(pat)
    {
    }
  };

  class radial_gradient_pattern : public gradient_pattern {
  public:
    static radial_gradient_pattern create(double cx0, double cy0, double radius0, double cx1, double cy1, double radius1) { return cairo_pattern_create_radial(cx0, cy0, radius0, cx1, cy1, radius1); }
    static radial_gradient_pattern create(vec2f center0, double radius0, vec2f center1, double radius1) { return cairo_pattern_create_radial(center0.x, center0.y, radius0, center1.x, center1.y, radius1); }

    std::tuple<vec2f, double, vec2f, double> get_radial_circles()
    {
      vec2f center0;
      double radius0;
      vec2f center1;
      double radius1;
      [[maybe_unused]] auto result = cairo_pattern_get_radial_circles(m_pattern, &center0.x, &center0.y, &radius0, &center1.x, &center1.y, &radius1);
      assert(result == CAIRO_STATUS_SUCCESS);
      return { center0, radius0, center1, radius1 };
    }

  private:
    radial_gradient_pattern(cairo_pattern_t* pat)
    : gradient_pattern(pat)
    {
    }
  };

  class mesh_pattern : public pattern {
  public:
    static mesh_pattern create() { return cairo_pattern_create_mesh(); }

    void begin_patch() { cairo_mesh_pattern_begin_patch(m_pattern); }
    void end_patch() { cairo_mesh_pattern_end_patch(m_pattern); }
    void move_to(double x, double y) { cairo_mesh_pattern_move_to(m_pattern, x, y); }
    void move_to(vec2f point) { cairo_mesh_pattern_move_to(m_pattern, point.x, point.y); }
    void line_to(double x, double y) { cairo_mesh_pattern_line_to(m_pattern, x, y); }
    void line_to(vec2f point) { cairo_mesh_pattern_line_to(m_pattern, point.x, point.y); }
    void curve_to(double x1, double y1, double x2, double y2, double x3, double y3) { cairo_mesh_pattern_curve_to(m_pattern, x1, y1, x2, y2, x3, y3); }
    void curve_to(vec2f p1, vec2f p2, vec2f p3) { cairo_mesh_pattern_curve_to(m_pattern, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y); }

    void set_control_point(unsigned point_num, double x, double y) { cairo_mesh_pattern_set_control_point(m_pattern, point_num, x, y); }
    void set_control_point(unsigned point_num, vec2f point) { cairo_mesh_pattern_set_control_point(m_pattern, point_num, point.x, point.y); }
    void set_corner_color_rgb(unsigned corner_num, double red, double green, double blue) { cairo_mesh_pattern_set_corner_color_rgb(m_pattern, corner_num, red, green, blue); }
    void set_corner_color_rgba(unsigned corner_num, double red, double green, double blue, double alpha) { cairo_mesh_pattern_set_corner_color_rgba(m_pattern, corner_num, red, green, blue, alpha); }
    void set_corner_color(unsigned corner_num, color col) { cairo_mesh_pattern_set_corner_color_rgba(m_pattern, corner_num, col.r, col.g, col.b, col.a); }

    unsigned get_patch_count()
    {
      unsigned count;
      [[maybe_unused]] auto result = cairo_mesh_pattern_get_patch_count(m_pattern, &count);
      assert(result == CAIRO_STATUS_SUCCESS);
      return count;
    }

    path get_path(unsigned patch_num) { return cairo_mesh_pattern_get_path(m_pattern, patch_num); }

    color get_corner_color(unsigned patch_num, unsigned corner_num)
    {
      color col;
      [[maybe_unused]] auto result = cairo_mesh_pattern_get_corner_color_rgba(m_pattern, patch_num, corner_num, &col.r, &col.g, &col.b, &col.a);
      assert(result == CAIRO_STATUS_SUCCESS);
      return col;
    }

    vec2f get_control_point(unsigned patch_num, unsigned point_num)
    {
      vec2f point;
      [[maybe_unused]] auto result = cairo_mesh_pattern_get_control_point(m_pattern, patch_num, point_num, &point.x, &point.y);
      assert(result == CAIRO_STATUS_SUCCESS);
      return point;
    }

  private:
    mesh_pattern(cairo_pattern_t* pat)
    : pattern(pat)
    {
    }
  };

  /*
   * surfaces
   */

  inline int format_stride_for_width(format fmt, int width) { return cairo_format_stride_for_width(static_cast<cairo_format_t>(fmt), width); }

  class image_surface : public surface {
  public:
    static image_surface create(format fmt, int width, int height) { return cairo_image_surface_create(static_cast<cairo_format_t>(fmt), width, height); }
    static image_surface create(format fmt, vec2i size) { return cairo_image_surface_create(static_cast<cairo_format_t>(fmt), size.x, size.y); }
    static image_surface create_for_data(unsigned char* data, format fmt, int width, int height, int stride) { return cairo_image_surface_create_for_data(data, static_cast<cairo_format_t>(fmt), width, height, stride); }
    static image_surface create_for_data(unsigned char* data, format fmt, vec2i size, int stride) { return cairo_image_surface_create_for_data(data, static_cast<cairo_format_t>(fmt), size.x, size.y, stride); }

#if CAIRO_HAS_PNG_FUNCTIONS
    static image_surface create_from_png(const char* filename) { return cairo_image_surface_create_from_png(filename);  }
    static image_surface create_from_png(const std::filesystem::path& filename) { return cairo_image_surface_create_from_png(filename.string().c_str());  }
#endif

    unsigned char* get_data() { return cairo_image_surface_get_data(m_surface); }
    format get_format() { return static_cast<format>(cairo_image_surface_get_format(m_surface)); }
    int get_width() { return cairo_image_surface_get_width(m_surface); }
    int get_height() { return cairo_image_surface_get_height(m_surface); }
    int get_stride() { return cairo_image_surface_get_stride(m_surface); }


  private:
    image_surface(cairo_surface_t* surf)
    : surface(surf)
    {
    }
  };

  class recording_surface : public surface {
  public:
    static recording_surface create(content cnt, rectf extents) { const cairo_rectangle_t rectangle = { extents.x, extents.y, extents.w, extents.h }; return cairo_recording_surface_create(static_cast<cairo_content_t>(cnt), &rectangle); }

    rectf get_ink_extents() { rectf extents; cairo_recording_surface_ink_extents(m_surface, &extents.x, &extents.y, &extents.w, &extents.h); return extents; }
    std::pair<bool, rectf> get_extents() { cairo_rectangle_t extents; auto ret = cairo_recording_surface_get_extents(m_surface, &extents); return { ret != 0, { extents.x, extents.y, extents.width, extents.height }}; }

  private:
    recording_surface(cairo_surface_t* surf)
    : surface(surf)
    {
    }
  };

#if CAIRO_HAS_PDF_SURFACE

  enum class pdf_version : std::underlying_type_t<cairo_pdf_version_t> {
    v_1_4,
    v_1_5,
  };

  inline std::string_view to_string(pdf_version version) { return cairo_pdf_version_to_string(static_cast<cairo_pdf_version_t>(version)); }

  enum class pdf_outline_flags : std::underlying_type_t<cairo_pdf_outline_flags_t> {
    open = CAIRO_PDF_OUTLINE_FLAG_OPEN,
    bold = CAIRO_PDF_OUTLINE_FLAG_BOLD,
    italic = CAIRO_PDF_OUTLINE_FLAG_ITALIC,
  };

  inline constexpr int pdf_outline_root = CAIRO_PDF_OUTLINE_ROOT;

  enum class pdf_metadata : std::underlying_type_t<cairo_pdf_metadata_t> {
    title = CAIRO_PDF_METADATA_TITLE,
    author = CAIRO_PDF_METADATA_AUTHOR,
    subject = CAIRO_PDF_METADATA_SUBJECT,
    keywords = CAIRO_PDF_METADATA_KEYWORDS,
    creator = CAIRO_PDF_METADATA_CREATOR,
    date = CAIRO_PDF_METADATA_CREATE_DATE,
    mod_date = CAIRO_PDF_METADATA_MOD_DATE,
  };

  class pdf_surface : public surface {
  public:
    static pdf_surface create(const char* filename, double width_in_points, double height_in_points) { return cairo_pdf_surface_create(filename, width_in_points, height_in_points); }
    static pdf_surface create(const std::filesystem::path& filename, double width_in_points, double height_in_points) { return cairo_pdf_surface_create(filename.string().c_str(), width_in_points, height_in_points); }

    void restrict_to_version(pdf_version version) { cairo_pdf_surface_restrict_to_version(m_surface, static_cast<cairo_pdf_version_t>(version)); }

    void set_size(double width_in_points, double height_in_points) {cairo_pdf_surface_set_size(m_surface, width_in_points, height_in_points); }
    void set_size(vec2f size_in_points) {cairo_pdf_surface_set_size(m_surface, size_in_points.x, size_in_points.y); }

    void add_outline(int parent_id, const char* utf8, const char* link_attribs, pdf_outline_flags flags) { cairo_pdf_surface_add_outline(m_surface, parent_id, utf8, link_attribs, static_cast<cairo_pdf_outline_flags_t>(flags)); }

    void set_metadata(pdf_metadata metadata, const char* utf8) { cairo_pdf_surface_set_metadata(m_surface, static_cast<cairo_pdf_metadata_t>(metadata), utf8); }
    void set_page_label(const char* utf8) { cairo_pdf_surface_set_page_label(m_surface, utf8); }
    void set_thumbnail_size(int width, int height) { cairo_pdf_surface_set_thumbnail_size(m_surface, width, height); }
    void set_thumbnail_size(vec2i size) { cairo_pdf_surface_set_thumbnail_size(m_surface, size.x, size.y); }

    static std::vector<pdf_version> get_versions()
    {
      const cairo_pdf_version_t *raw_versions;
      int raw_num_versions;
      cairo_pdf_get_versions(&raw_versions, &raw_num_versions);

      std::vector<pdf_version> versions;

      for (int i = 0; i < raw_num_versions; ++i) {
        versions.push_back(static_cast<pdf_version>(raw_versions[i]));
      }

      return versions;
    }

  private:
    pdf_surface(cairo_surface_t* surf)
    : surface(surf)
    {
    }
  };

#endif

  inline void debug_reset_static_data() { cairo_debug_reset_static_data(); }
}

#endif // CAIROPP_H
