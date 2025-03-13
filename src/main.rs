use tokio_test;
use yahoo_finance_api as yahoo;
use yahoo_finance_api::time::{Date, Month, OffsetDateTime, UtcOffset};

fn main() {
    let provider = yahoo::YahooConnector::new().unwrap();

    // 날짜 범위를 2년으로 확장
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

    // SCHD 배당금 정보
    println!("SCHD 배당금 정보:");
    let response =
        tokio_test::block_on(provider.get_quote_history("SPY", start_date, end_date)).unwrap();
    let dividends = response.dividends().unwrap();
    for dividend in dividends {
        let timestamp = dividend.date;
        let datetime = OffsetDateTime::from_unix_timestamp(timestamp as i64).unwrap();
        println!(
            "배당금: ${:.4}, 날짜: {}-{:02}-{:02}",
            dividend.amount,
            datetime.year(),
            u8::from(datetime.month()),
            datetime.day()
        );
    }

    // 다른 ETF와 비교 (예: SPY)
    println!("\nSPY 배당금 정보:");
    let spy_response =
        tokio_test::block_on(provider.get_quote_history("SPY", start_date, end_date)).unwrap();
    let spy_dividends = spy_response.dividends().unwrap();
    for dividend in spy_dividends {
        let timestamp = dividend.date;
        let datetime = OffsetDateTime::from_unix_timestamp(timestamp as i64).unwrap();
        println!(
            "배당금: ${:.4}, 날짜: {}-{:02}-{:02}",
            dividend.amount,
            datetime.year(),
            u8::from(datetime.month()),
            datetime.day()
        );
    }
}
