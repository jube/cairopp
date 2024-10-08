// This file is in the public domain
#include <cairopp.h>

#include <cmath>

#include <iostream>

namespace {

  /*
   * Adapted from: https://www.cairographics.org/samples/
   * Image from: https://www.publicdomainpictures.net/en/view-image.php?image=211882&picture=landscape-with-a-lake
   */

  constexpr cairo::Vec2I SIZE = { 512, 512 };

  constexpr double PI = 3.141592653589793238462643383279502884197169399;

  void arc(cairo::Context& ctx)
  {
    double xc = 128.0;
    double yc = 128.0;
    double radius = 100.0;
    double angle1 = 45.0  * (PI / 180.0);  /* angles are specified */
    double angle2 = 180.0 * (PI / 180.0);  /* in radians           */

    ctx.set_line_width(10.0);
    ctx.arc(xc, yc, radius, angle1, angle2);
    ctx.stroke();

    /* draw helping lines */
    ctx.set_source_rgba(1, 0.2, 0.2, 0.6);
    ctx.set_line_width(6.0);

    ctx.arc(xc, yc, 10.0, 0, 2 * PI);
    ctx.fill();

    ctx.arc(xc, yc, radius, angle1, angle1);
    ctx.line_to(xc, yc).arc(xc, yc, radius, angle2, angle2).line_to(xc, yc);
    ctx.stroke();
  }

  void arc_negative(cairo::Context& ctx)
  {
    double xc = 128.0;
    double yc = 128.0;
    double radius = 100.0;
    double angle1 = 45.0  * (PI / 180.0);  /* angles are specified */
    double angle2 = 180.0 * (PI / 180.0);  /* in radians           */

    ctx.set_line_width(10.0);
    ctx.arc_negative(xc, yc, radius, angle1, angle2);
    ctx.stroke();

    /* draw helping lines */
    ctx.set_source_rgba(1, 0.2, 0.2, 0.6);
    ctx.set_line_width(6.0);

    ctx.arc(xc, yc, 10.0, 0, 2 * PI);
    ctx.fill();

    ctx.arc(xc, yc, radius, angle1, angle1);
    ctx.line_to(xc, yc).arc(xc, yc, radius, angle2, angle2).line_to(xc, yc);
    ctx.stroke();
  }

  void clip(cairo::Context& ctx)
  {
    ctx.arc(128.0, 128.0, 76.8, 0, 2 * PI);
    ctx.clip();

    ctx.new_path();  /* current path is not consumed by clip() */
    ctx.rectangle(0, 0, 256, 256);
    ctx.fill();
    ctx.set_source_rgb(0, 1, 0);
    ctx.move_to(0, 0).line_to(256, 256).move_to(256, 0).line_to(0, 256);
    ctx.set_line_width(10.0);
    ctx.stroke();
  }

  void clip_image(cairo::Context& ctx)
  {
    ctx.arc(128.0, 128.0, 76.8, 0, 2 * PI);
    ctx.clip();
    ctx.new_path(); /* path not consumed by clip() */

    auto image = cairo::ImageSurface::create_from_png("images/landscape-with-a-lake.png");
    int w = image.width();
    int h = image.height();

    ctx.scale(256.0/w, 256.0/h);

    ctx.set_source(image, 0, 0);
    ctx.paint();
  }

  void curve_rectangle(cairo::Context& ctx)
  {
    /* a custom shape that could be wrapped in a function */
    double x0 = 25.6; /* parameters like rectangle() */
    double y0 = 25.6;
    double rect_width = 204.8;
    double rect_height = 204.8;
    double radius = 102.4; /* and an approximate curvature radius */

    double x1 = x0 + rect_width;
    double y1 = y0 + rect_height;

    if (rect_width / 2 < radius) {
      if (rect_height / 2 < radius) {
        ctx.move_to(x0, (y0 + y1)/2);
        ctx.curve_to(x0, y0, x0, y0, (x0 + x1) / 2, y0);
        ctx.curve_to(x1, y0, x1, y0, x1, (y0 + y1) / 2);
        ctx.curve_to(x1, y1, x1, y1, (x1 + x0) / 2, y1);
        ctx.curve_to(x0, y1, x0, y1, x0, (y0 + y1) / 2);
      } else {
        ctx.move_to(x0, y0 + radius);
        ctx.curve_to(x0, y0, x0, y0, (x0 + x1) / 2, y0);
        ctx.curve_to(x1, y0, x1, y0, x1, y0 + radius);
        ctx.line_to(x1, y1 - radius);
        ctx.curve_to(x1, y1, x1, y1, (x1 + x0) / 2, y1);
        ctx.curve_to(x0, y1, x0, y1, x0, y1 - radius);
      }
    } else {
      if (rect_height / 2 < radius) {
        ctx.move_to(x0, (y0 + y1) / 2);
        ctx.curve_to(x0, y0, x0 , y0, x0 + radius, y0);
        ctx.line_to(x1 - radius, y0);
        ctx.curve_to(x1, y0, x1, y0, x1, (y0 + y1)/2);
        ctx.curve_to(x1, y1, x1, y1, x1 - radius, y1);
        ctx.line_to(x0 + radius, y1);
        ctx.curve_to(x0, y1, x0, y1, x0, (y0 + y1) / 2);
      } else {
        ctx.move_to (x0, y0 + radius);
        ctx.curve_to(x0, y0, x0 , y0, x0 + radius, y0);
        ctx.line_to(x1 - radius, y0);
        ctx.curve_to(x1, y0, x1, y0, x1, y0 + radius);
        ctx.line_to(x1, y1 - radius);
        ctx.curve_to(x1, y1, x1, y1, x1 - radius, y1);
        ctx.line_to(x0 + radius, y1);
        ctx.curve_to(x0, y1, x0, y1, x0, y1 - radius);
      }
    }

    ctx.close_path();

    ctx.set_source_rgb(0.5, 0.5, 1);
    ctx.fill_preserve();
    ctx.set_source_rgba(0.5, 0, 0, 0.5);
    ctx.set_line_width(10.0);
    ctx.stroke();
  }

  void curve_to(cairo::Context& ctx)
  {
    double x = 25.6;
    double y = 128.0;

    double x1 = 102.4;
    double y1 = 230.4;
    double x2 = 153.6;
    double y2 = 25.6;
    double x3 = 230.4;
    double y3 = 128.0;

    ctx.move_to(x, y);
    ctx.curve_to(x1, y1, x2, y2, x3, y3);

    ctx.set_line_width(10.0);
    ctx.stroke();

    ctx.set_source_rgba(1.0, 0.2, 0.2, 0.6);
    ctx.set_line_width(6.0);
    ctx.move_to(x, y).line_to(x1, y1).move_to(x2, y2).line_to(x3, y3);
    ctx.stroke();
  }

  void dash(cairo::Context& ctx)
  {
    double dashes[] = { 50.0, 10.0, 10.0, 10.0 };
    double offset = -50.0;

    ctx.set_dash(dashes, offset);
    ctx.set_line_width(10.0);

    ctx.move_to(128.0, 25.6).line_to(230.4, 230.4).rel_line_to(-102.4, 0.0).curve_to(51.2, 230.4, 51.2, 128.0, 128.0, 128.0);
    ctx.stroke();
  }

  void fill_and_stroke(cairo::Context& cr)
  {
    cr.move_to(128.0, 25.6);
    cr.line_to(230.4, 230.4);
    cr.rel_line_to(-102.4, 0.0);
    cr.curve_to(51.2, 230.4, 51.2, 128.0, 128.0, 128.0);
    cr.close_path();

    cr.move_to(64.0, 25.6);
    cr.rel_line_to(51.2, 51.2);
    cr.rel_line_to(-51.2, 51.2);
    cr.rel_line_to(-51.2, -51.2);
    cr.close_path();

    cr.set_line_width(10.0);
    cr.set_source_rgb(0, 0, 1);
    cr.fill_preserve();
    cr.set_source_rgb(0, 0, 0);
    cr.stroke();
  }

  void fill_style(cairo::Context& ctx)
  {
    ctx.set_line_width(6);

    ctx.rectangle(12, 12, 232, 70);
    ctx.new_sub_path(); ctx.arc(64, 64, 40, 0, 2 * PI);
    ctx.new_sub_path(); ctx.arc_negative(192, 64, 40, 0, -2 * PI);

    ctx.set_fill_rule(cairo::FillRule::EvenOdd);
    ctx.set_source_rgb(0, 0.7, 0); ctx.fill_preserve();
    ctx.set_source_rgb(0, 0, 0); ctx.stroke();

    ctx.translate(0, 128);
    ctx.rectangle(12, 12, 232, 70);
    ctx.new_sub_path(); ctx.arc(64, 64, 40, 0, 2 * PI);
    ctx.new_sub_path(); ctx.arc_negative(192, 64, 40, 0, -2 * PI);

    ctx.set_fill_rule(cairo::FillRule::Winding);
    ctx.set_source_rgb(0, 0, 0.9); ctx.fill_preserve();
    ctx.set_source_rgb(0, 0, 0); ctx.stroke();
  }

  void gradient(cairo::Context& ctx)
  {
    {
      auto pat = cairo::LinearGradientPattern::create(0.0, 0.0, 0.0, 256.0);
      pat.add_color_stop_rgba(1, 0, 0, 0, 1);
      pat.add_color_stop_rgba(0, 1, 1, 1, 1);

      ctx.rectangle(0, 0, 256, 256);
      ctx.set_source(pat);
      ctx.fill();
    }

    {
      auto pat = cairo::RadialGradientPattern::create(115.2, 102.4, 25.6, 102.4, 102.4, 128.0);
      pat.add_color_stop_rgba(0, 1, 1, 1, 1);
      pat.add_color_stop_rgba(1, 0, 0, 0, 1);

      ctx.set_source(pat);
      ctx.arc(128.0, 128.0, 76.8, 0, 2 * PI);
      ctx.fill();
    }
  }

  void image(cairo::Context& ctx)
  {
    auto image = cairo::ImageSurface::create_from_png("images/landscape-with-a-lake.png");
    int w = image.width();
    int h = image.height();

    ctx.translate(128.0, 128.0);
    ctx.rotate(45 * PI / 180);
    ctx.scale (256.0 / w, 256.0 / h);
    ctx.translate(-0.5 * w, -0.5 * h);

    ctx.set_source(image, 0, 0);
    ctx.paint();
  }

  void image_pattern(cairo::Context& ctx)
  {
    auto image = cairo::ImageSurface::create_from_png("images/landscape-with-a-lake.png");
    int w = image.width();
    int h = image.height();

    auto pattern = cairo::SurfacePattern::create(image);
    pattern.set_extend(cairo::Extend::Repeat);

    ctx.translate(128.0, 128.0);
    ctx.rotate(PI / 4);
    ctx.scale(1 / std::sqrt(2), 1 / std::sqrt(2));
    ctx.translate(-128.0, -128.0);

    auto matrix = cairo::Matrix::create_scale(w / 256.0 * 5.0, h / 256.0 * 5.0);
    pattern.set_matrix(matrix);

    ctx.set_source(pattern);

    ctx.rectangle(0, 0, 256.0, 256.0);
    ctx.fill();
  }

  void multi_segments_cap(cairo::Context& ctx)
  {
    ctx.move_to(50.0, 75.0);
    ctx.line_to(200.0, 75.0);

    ctx.move_to(50.0, 125.0);
    ctx.line_to(200.0, 125.0);

    ctx.move_to(50.0, 175.0);
    ctx.line_to(200.0, 175.0);

    ctx.set_line_width(30.0);
    ctx.set_line_cap(cairo::LineCap::Round);
    ctx.stroke();
  }

  void rounded_rectangle(cairo::Context& ctx)
  {
    /* a custom shape that could be wrapped in a function */
    double x = 25.6; /* parameters like rectangle() */
    double y = 25.6;
    double width = 204.8;
    double height = 204.8;
    double aspect = 1.0; /* aspect ratio */
    double corner_radius = height / 10.0; /* and corner curvature radius */

    double radius = corner_radius / aspect;
    constexpr double Degrees = PI / 180.0;

    ctx.new_sub_path();
    ctx.arc(x + width - radius, y + radius, radius, -90 * Degrees, 0 * Degrees);
    ctx.arc(x + width - radius, y + height - radius, radius, 0 * Degrees, 90 * Degrees);
    ctx.arc(x + radius, y + height - radius, radius, 90 * Degrees, 180 * Degrees);
    ctx.arc(x + radius, y + radius, radius, 180 * Degrees, 270 * Degrees);
    ctx.close_path();

    ctx.set_source_rgb(0.5, 0.5, 1);
    ctx.fill_preserve();
    ctx.set_source_rgba(0.5, 0, 0, 0.5);
    ctx.set_line_width(10.0);
    ctx.stroke();
  }

  void set_line_cap(cairo::Context& ctx)
  {
    ctx.set_line_width(30.0);
    ctx.set_line_cap(cairo::LineCap::Butt); /* default */
    ctx.move_to(64.0, 50.0); ctx.line_to(64.0, 200.0);
    ctx.stroke();
    ctx.set_line_cap(cairo::LineCap::Round);
    ctx.move_to(128.0, 50.0); ctx.line_to(128.0, 200.0);
    ctx.stroke();
    ctx.set_line_cap(cairo::LineCap::Square);
    ctx.move_to(192.0, 50.0); ctx.line_to(192.0, 200.0);
    ctx.stroke();

    /* draw helping lines */
    ctx.set_source_rgb(1, 0.2, 0.2);
    ctx.set_line_width(2.56);
    ctx.move_to(64.0, 50.0); ctx.line_to(64.0, 200.0);
    ctx.move_to(128.0, 50.0); ctx.line_to(128.0, 200.0);
    ctx.move_to(192.0, 50.0); ctx.line_to(192.0, 200.0);
    ctx.stroke();
  }

  void set_line_join(cairo::Context& ctx)
  {
    ctx.set_line_width(40.96);
    ctx.move_to(76.8, 84.48);
    ctx.rel_line_to(51.2, -51.2);
    ctx.rel_line_to(51.2, 51.2);
    ctx.set_line_join(cairo::LineJoin::Miter); /* default */
    ctx.stroke();

    ctx.move_to(76.8, 161.28);
    ctx.rel_line_to(51.2, -51.2);
    ctx.rel_line_to(51.2, 51.2);
    ctx.set_line_join(cairo::LineJoin::Bevel);
    ctx.stroke();

    ctx.move_to(76.8, 238.08);
    ctx.rel_line_to(51.2, -51.2);
    ctx.rel_line_to(51.2, 51.2);
    ctx.set_line_join(cairo::LineJoin::Round);
    ctx.stroke();
  }

  void text(cairo::Context& ctx)
  {
    ctx.select_font_face("Sans", cairo::FontSlant::Normal, cairo::FontWeight::Bold);
    ctx.set_font_size(90.0);

    ctx.move_to(10.0, 135.0);
    ctx.show_text("Hello");

    ctx.move_to(70.0, 165.0);
    ctx.text_path("void");
    ctx.set_source_rgb(0.5, 0.5, 1);
    ctx.fill_preserve();
    ctx.set_source_rgb(0, 0, 0);
    ctx.set_line_width(2.56);
    ctx.stroke();

    /* draw helping lines */
    ctx.set_source_rgba(1, 0.2, 0.2, 0.6);
    ctx.arc(10.0, 135.0, 5.12, 0, 2 * PI);
    ctx.close_path();
    ctx.arc(70.0, 165.0, 5.12, 0, 2 * PI);
    ctx.fill();
  }

  void text_align_center(cairo::Context& ctx)
  {
    const char* utf8 = "cairo";

    ctx.select_font_face("Sans", cairo::FontSlant::Normal, cairo::FontWeight::Normal);

    ctx.set_font_size(52.0);
    auto extents = ctx.text_extents(utf8);
    double x = 128.0 - (extents.width / 2 + extents.x_bearing);
    double y = 128.0 - (extents.height / 2 + extents.y_bearing);

    ctx.move_to(x, y);
    ctx.show_text(utf8);

    /* draw helping lines */
    ctx.set_source_rgba(1, 0.2, 0.2, 0.6);
    ctx.set_line_width(6.0);
    ctx.arc(x, y, 10.0, 0, 2 * PI);
    ctx.fill();
    ctx.move_to(128.0, 0);
    ctx.rel_line_to(0, 256);
    ctx.move_to(0, 128.0);
    ctx.rel_line_to(256, 0);
    ctx.stroke();
  }

  void text_extents(cairo::Context& ctx)
  {
    const char* utf8 = "cairo";

    ctx.select_font_face("Sans", cairo::FontSlant::Normal, cairo::FontWeight::Normal);

    ctx.set_font_size(100.0);
    auto extents = ctx.text_extents(utf8);

    double x = 25.0;
    double y = 150.0;

    ctx.move_to(x,y);
    ctx.show_text(utf8);

    /* draw helping lines */
    ctx.set_source_rgba(1, 0.2, 0.2, 0.6);
    ctx.set_line_width(6.0);
    ctx.arc(x, y, 10.0, 0, 2 * PI);
    ctx.fill();
    ctx.move_to(x,y);
    ctx.rel_line_to(0, -extents.height);
    ctx.rel_line_to(extents.width, 0);
    ctx.rel_line_to(extents.x_bearing, -extents.y_bearing);
    ctx.stroke();
  }

  using SampleFunc = void (*)(cairo::Context&);

  struct Sample {
    const char* name;
    SampleFunc func;
  };

  constexpr Sample SAMPLES[] = {
    { "arc", arc },
    { "arc_negative", arc_negative },
    { "clip", clip },
    { "clip_image", clip_image },
    { "curve_rectangle", curve_rectangle },
    { "curve_to", curve_to },
    { "dash", dash },
    { "fill_and_stroke", fill_and_stroke },
    { "fill_style", fill_style },
    { "gradient", gradient },
    { "image", image },
    { "image_pattern", image_pattern },
    { "multi_segments_cap", multi_segments_cap },
    { "rounded_rectangle", rounded_rectangle },
    { "set_line_cap", set_line_cap },
    { "set_line_join", set_line_join },
    { "text", text },
    { "text_align_center", text_align_center },
    { "text_extents", text_extents },
  };

}

int main()
{
  for (Sample sample : SAMPLES) {
    std::cout << "Executing '" << sample.name << "'...\n";

    std::filesystem::path filename = sample.name;
    filename.replace_extension(".png");

    cairo::ImageSurface surface = cairo::ImageSurface::create(cairo::Format::Argb32, SIZE);
    cairo::Context context(surface);
    context.scale(2.0, 2.0);

    {
      cairo::Subcontext sub(context);
      context.set_source_rgb(0.95, 0.95, 0.95);
      context.paint();
    }

    sample.func(context);
    surface.write_to_png(filename);
  }

  cairo::debug_reset_static_data();
}
