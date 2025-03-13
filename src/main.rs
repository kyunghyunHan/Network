use chrono::NaiveDateTime;
use plotters::prelude::*;
use plotters::series::{LineSeries, PointSeries};
use std::error::Error;
use yahoo_finance_api as yahoo;
use yahoo_finance_api::time::{Date, Month, OffsetDateTime};

const OUT_FILE_NAME: &str = "./assets/img/dividends.png";

pub fn main() -> Result<(), Box<dyn Error>> {
    // Connect to Yahoo Finance API
    let provider = yahoo::YahooConnector::new()?;

    // Define date range (2000-2024)
    let start_date = Date::from_calendar_date(2000, Month::January, 1)
        .unwrap()
        .with_hms(0, 0, 0)
        .unwrap()
        .assume_utc();

    let end_date = Date::from_calendar_date(2024, Month::December, 31)
        .unwrap()
        .with_hms(0, 0, 0)
        .unwrap()
        .assume_utc();

    // Get SPY data
    let spy_response = tokio_test::block_on(provider.get_quote_history("SPY", start_date, end_date))?;
    let spy_quotes = spy_response.quotes()?;
    let spy_dividends = spy_response.dividends()?;

    // Get SCHD data
    let schd_response = tokio_test::block_on(provider.get_quote_history("SCHD", start_date, end_date))?;
    let schd_quotes = schd_response.quotes()?;
    let schd_dividends = schd_response.dividends()?;

    println!("SPY quotes: {}, dividends: {}", spy_quotes.len(), spy_dividends.len());
    println!("SCHD quotes: {}, dividends: {}", schd_quotes.len(), schd_dividends.len());

    // Setup visualization
    let root = BitMapBackend::new(OUT_FILE_NAME, (1200, 800)).into_drawing_area();
    root.fill(&WHITE)?;

    // Split the drawing area into two vertically
    let (price_area, dividend_area) = root.split_vertically(400);

    // Convert data for plotting
    // Prices
    let spy_price_data: Vec<(f64, f64)> = spy_quotes.iter()
        .map(|quote| (quote.timestamp as f64, quote.close))
        .collect();

    let schd_price_data: Vec<(f64, f64)> = schd_quotes.iter()
        .map(|quote| (quote.timestamp as f64, quote.close))
        .collect();

    // Dividends
    let spy_dividend_data: Vec<(f64, f64)> = spy_dividends.iter()
        .map(|div| (div.date as f64, div.amount))
        .collect();

    let schd_dividend_data: Vec<(f64, f64)> = schd_dividends.iter()
        .map(|div| (div.date as f64, div.amount))
        .collect();

    // Find min/max values for scaling
    let min_time = spy_quotes.first().map(|q| q.timestamp).unwrap_or(0) as f64;
    let max_time = spy_quotes.last().map(|q| q.timestamp).unwrap_or(0) as f64;
    
    let min_price = spy_quotes.iter()
        .map(|q| q.low)
        .min_by(|a, b| a.partial_cmp(b).unwrap())
        .unwrap_or(0.0)
        .min(schd_quotes.iter()
            .map(|q| q.low)
            .min_by(|a, b| a.partial_cmp(b).unwrap())
            .unwrap_or(0.0));
    
    let max_price = spy_quotes.iter()
        .map(|q| q.high)
        .max_by(|a, b| a.partial_cmp(b).unwrap())
        .unwrap_or(0.0)
        .max(schd_quotes.iter()
            .map(|q| q.high)
            .max_by(|a, b| a.partial_cmp(b).unwrap())
            .unwrap_or(0.0));

    let min_div = spy_dividends.iter()
        .map(|d| d.amount)
        .min_by(|a, b| a.partial_cmp(b).unwrap())
        .unwrap_or(0.0)
        .min(schd_dividends.iter()
            .map(|d| d.amount)
            .min_by(|a, b| a.partial_cmp(b).unwrap())
            .unwrap_or(0.0));
    
    let max_div = spy_dividends.iter()
        .map(|d| d.amount)
        .max_by(|a, b| a.partial_cmp(b).unwrap())
        .unwrap_or(0.0)
        .max(schd_dividends.iter()
            .map(|d| d.amount)
            .max_by(|a, b| a.partial_cmp(b).unwrap())
            .unwrap_or(0.0));

    // Create price chart
    let mut price_chart = ChartBuilder::on(&price_area)
        .caption("Stock Prices", ("sans-serif", 30))
        .margin(5)
        .x_label_area_size(40)
        .y_label_area_size(50)
        .build_cartesian_2d(min_time..max_time, min_price * 0.9..max_price * 1.1)?;

    price_chart
        .configure_mesh()
        .x_labels(20)
        .x_label_formatter(&|x| {
            let timestamp = *x as i64;
            let datetime = OffsetDateTime::from_unix_timestamp(timestamp).unwrap();
            format!("{}-{:02}", datetime.year(), u8::from(datetime.month()))
        })
        .draw()?;

    // Draw price lines
    price_chart
        .draw_series(LineSeries::new(spy_price_data, &BLUE))?
        .label("SPY")
        .legend(|(x, y)| PathElement::new(vec![(x, y), (x + 20, y)], &BLUE));

    price_chart
        .draw_series(LineSeries::new(schd_price_data, &RED))?
        .label("SCHD")
        .legend(|(x, y)| PathElement::new(vec![(x, y), (x + 20, y)], &RED));

    price_chart.configure_series_labels()
        .background_style(&WHITE.mix(0.8))
        .border_style(&BLACK)
        .draw()?;

    // Create dividend chart
    let mut div_chart = ChartBuilder::on(&dividend_area)
        .caption("Dividend History", ("sans-serif", 30))
        .margin(5)
        .x_label_area_size(40)
        .y_label_area_size(50)
        .build_cartesian_2d(min_time..max_time, min_div * 0.9..max_div * 1.1)?;

    div_chart
        .configure_mesh()
        .x_labels(20)
        .x_label_formatter(&|x| {
            let timestamp = *x as i64;
            let datetime = OffsetDateTime::from_unix_timestamp(timestamp).unwrap();
            format!("{}-{:02}", datetime.year(), u8::from(datetime.month()))
        })
        .draw()?;

    // Draw dividend lines
    div_chart
        .draw_series(LineSeries::new(
            spy_dividend_data,
            &BLUE
        ))?
        .label("SPY Dividends")
        .legend(|(x, y)| PathElement::new(vec![(x, y), (x + 20, y)], &BLUE));

    div_chart
        .draw_series(LineSeries::new(
            schd_dividend_data,
            &RED
        ))?
        .label("SCHD Dividends")
        .legend(|(x, y)| PathElement::new(vec![(x, y), (x + 20, y)], &RED));

    div_chart.configure_series_labels()
        .background_style(&WHITE.mix(0.8))
        .border_style(&BLACK)
        .draw()?;

    root.present()?;
    println!("Chart has been saved to {}", OUT_FILE_NAME);

    Ok(())
}