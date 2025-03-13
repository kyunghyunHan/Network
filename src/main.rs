use reqwest;
use serde_json::Value;
use tokio;

#[tokio::main]
async fn main() {
    let mut usa_all_stocks_list = Vec::new();

    // 거래소 목록
    let exchanges = ["NYSE", "NASDAQ", "AMEX"];

    for &exchange in &exchanges {
        if let Err(e) = fetch_stocks(exchange, &mut usa_all_stocks_list).await {
            eprintln!("Error fetching stocks for {}: {}", exchange, e);
        }
    }

    // 취합한 주식 정보 출력
    for stock in usa_all_stocks_list {
        println!("{:?}", stock);
    }
}

async fn fetch_stocks(exchange: &str, stock_list: &mut Vec<(String, String, String, f64, f64)>) -> Result<(), reqwest::Error> {
    let base_url = format!("https://api.stock.naver.com/stock/exchange/{}/marketValue", exchange);
    let response = reqwest::get(&base_url).await?.json::<Value>().await?;
    
    let total_count = response["totalCount"].as_u64().unwrap_or(0);
    let page_count = (total_count / 100) + 1;
    println!("{} pages for {}", page_count, exchange);

    for page in 1..=page_count {
        let page_url = format!("{}?page={}&pageSize=100", base_url, page);
        let response = reqwest::get(&page_url).await?.json::<Value>().await?;

        if let Some(stocks) = response["stocks"].as_array() {
            for stock in stocks {
                if stock["stockName"].as_str().unwrap() =="애플"{
                    println!("{}",stock);

                }
                // let reuters_code = stock["reutersCode"].as_str().unwrap_or("").to_string();
                // let symbol_code = stock["symbolCode"].as_str().unwrap_or("").to_string();
                // let stock_name = stock["stockName"].as_str().unwrap_or("").to_string();
                // let close_price = stock["closePrice"].as_f64().unwrap_or(0.0);
                // let dividend_yield = stock["dividend_info"].as_f64().unwrap_or(0.0); // 배당 수익률 추가

                // stock_list.push((reuters_code, symbol_code, stock_name, close_price, dividend_yield));
            }
        }
    }
    Ok(())
}
