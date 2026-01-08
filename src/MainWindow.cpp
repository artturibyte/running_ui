#include "MainWindow.h"
#include <iomanip>
#include <sstream>
#include <ctime>
#include <numeric>
#include <fstream>
#include <QWidget>
#include <QFrame>
#include <QStandardPaths>
#include <QDir>
#include <QDate>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Running Tracker");
    resize(500, 400);
    
    // Set up data file path
    QString data_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(data_dir);
    m_data_file = data_dir + "/running_data.txt";
    
    setup_ui();
    load_from_file();
    update_list_view();
    update_statistics();
}

void MainWindow::setup_ui() {
    // Create central widget and main layout
    QWidget *central_widget = new QWidget(this);
    QVBoxLayout *main_layout = new QVBoxLayout(central_widget);
    main_layout->setSpacing(10);
    main_layout->setContentsMargins(10, 10, 10, 10);
    
    // Input row
    QHBoxLayout *input_layout = new QHBoxLayout();
    input_layout->setSpacing(5);
    
    QLabel *date_label = new QLabel("Date:", this);
    m_date_edit = new QDateEdit(this);
    m_date_edit->setDate(QDate::currentDate());
    m_date_edit->setCalendarPopup(true);
    m_date_edit->setDisplayFormat("yyyy-MM-dd");
    
    QLabel *input_label = new QLabel("Kilometers:", this);
    m_kilometers_entry = new QLineEdit(this);
    m_kilometers_entry->setPlaceholderText("Enter kilometers (e.g., 5.5)");
    m_kilometers_entry->setMaxLength(10);
    
    m_add_button = new QPushButton("Add Entry", this);
    
    input_layout->addWidget(date_label);
    input_layout->addWidget(m_date_edit);
    input_layout->addWidget(input_label);
    input_layout->addWidget(m_kilometers_entry);
    input_layout->addWidget(m_add_button);
    
    // Statistics section
    QVBoxLayout *stats_layout = new QVBoxLayout();
    stats_layout->setSpacing(5);
    
    m_total_label = new QLabel("<b>Total: 0.0 km</b>", this);
    m_average_label = new QLabel("<b>Average: 0.0 km</b>", this);
    m_count_label = new QLabel("<b>Entries: 0</b>", this);
    m_daily_avg_label = new QLabel("<b>Daily Average: 0.0 km/day (Need: 2.7 km/day)</b>", this);
    m_goal_label = new QLabel("<b>Goal Progress: 0.0 / 1000 km (0%)</b>", this);
    
    m_total_label->setTextFormat(Qt::RichText);
    m_average_label->setTextFormat(Qt::RichText);
    m_count_label->setTextFormat(Qt::RichText);
    m_daily_avg_label->setTextFormat(Qt::RichText);
    m_goal_label->setTextFormat(Qt::RichText);
    
    stats_layout->addWidget(m_total_label);
    stats_layout->addWidget(m_average_label);
    stats_layout->addWidget(m_count_label);
    stats_layout->addWidget(m_daily_avg_label);
    stats_layout->addWidget(m_goal_label);
    
    // Separator
    QFrame *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    
    // List header
    QLabel *list_header = new QLabel("<b>Running History:</b>", this);
    list_header->setTextFormat(Qt::RichText);
    
    // List view
    m_list_view = new QTextEdit(this);
    m_list_view->setReadOnly(true);
    
    // Use monospace font for better alignment
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    m_list_view->setFont(font);
    
    // Add all widgets to main layout
    main_layout->addLayout(input_layout);
    main_layout->addLayout(stats_layout);
    main_layout->addWidget(separator);
    main_layout->addWidget(list_header);
    main_layout->addWidget(m_list_view);
    
    setCentralWidget(central_widget);
    
    // Connect signals
    connect(m_add_button, &QPushButton::clicked, this, &MainWindow::on_add_button_clicked);
    connect(m_kilometers_entry, &QLineEdit::returnPressed, this, &MainWindow::on_add_button_clicked);
}

void MainWindow::on_add_button_clicked() {
    QString input = m_kilometers_entry->text();
    
    if (input.isEmpty()) {
        return;
    }
    
    bool ok;
    double kilometers = input.toDouble(&ok);
    
    if (!ok) {
        QMessageBox::critical(this, "Error", "Invalid input! Please enter a valid number.");
        return;
    }
    
    if (kilometers <= 0) {
        QMessageBox::critical(this, "Error", "Please enter a positive number!");
        return;
    }
    
    // Get selected date
    QString date = m_date_edit->date().toString("yyyy-MM-dd");
    
    // Add new entry
    m_entries.emplace_back(date.toStdString(), kilometers);
    
    // Save to file
    save_to_file();
    
    // Update UI
    update_list_view();
    update_statistics();
    
    // Clear input field and reset date to today
    m_kilometers_entry->clear();
    m_date_edit->setDate(QDate::currentDate());
    m_kilometers_entry->setFocus();
}

void MainWindow::update_list_view() {
    std::ostringstream oss;
    
    if (m_entries.empty()) {
        oss << "No entries yet. Start tracking your runs!\n";
    } else {
        oss << std::left << std::setw(15) << "Date" 
            << std::right << std::setw(12) << "Kilometers\n";
        oss << std::string(27, '-') << "\n";
        
        for (const auto& entry : m_entries) {
            oss << std::left << std::setw(15) << entry.date
                << std::right << std::setw(10) << std::fixed << std::setprecision(2) 
                << entry.kilometers << " km\n";
        }
    }
    
    m_list_view->setPlainText(QString::fromStdString(oss.str()));
}

void MainWindow::update_statistics() {
    const double YEARLY_GOAL = 1000.0;
    const double REQUIRED_DAILY_AVG = YEARLY_GOAL / 365.0;
    
    if (m_entries.empty()) {
        m_total_label->setText("<b>Total: 0.0 km</b>");
        m_average_label->setText("<b>Average: 0.0 km</b>");
        m_count_label->setText("<b>Entries: 0</b>");
        
        std::ostringstream daily_oss;
        daily_oss << "<b>Daily Average: 0.0 km/day (Need: " 
                  << std::fixed << std::setprecision(1) << REQUIRED_DAILY_AVG << " km/day)</b>";
        m_daily_avg_label->setText(QString::fromStdString(daily_oss.str()));
        m_goal_label->setText("<b>Goal Progress: 0.0 / 1000 km (0%)</b>");
        return;
    }
    
    double total = std::accumulate(m_entries.begin(), m_entries.end(), 0.0,
                                   [](double sum, const RunningEntry& e) {
                                       return sum + e.kilometers;
                                   });
    
    double average = total / m_entries.size();
    
    // Calculate daily average based on date range
    int days_tracked = 1;
    if (m_entries.size() > 1) {
        // Find earliest and latest dates
        std::string earliest = m_entries[0].date;
        std::string latest = m_entries[0].date;
        
        for (const auto& entry : m_entries) {
            if (entry.date < earliest) earliest = entry.date;
            if (entry.date > latest) latest = entry.date;
        }
        
        // Parse dates and calculate difference
        QDate start = QDate::fromString(QString::fromStdString(earliest), "yyyy-MM-dd");
        QDate end = QDate::fromString(QString::fromStdString(latest), "yyyy-MM-dd");
        days_tracked = start.daysTo(end) + 1;
    }
    
    double daily_average = total / days_tracked;
    double progress_percent = (total / YEARLY_GOAL) * 100.0;
    double pace_difference = daily_average - REQUIRED_DAILY_AVG;
    
    std::ostringstream total_oss, avg_oss, count_oss, daily_oss, goal_oss;
    total_oss << "<b>Total: " << std::fixed << std::setprecision(2) << total << " km</b>";
    avg_oss << "<b>Average: " << std::fixed << std::setprecision(2) << average << " km</b>";
    count_oss << "<b>Entries: " << m_entries.size() << "</b>";
    
    daily_oss << "<b>Daily Average: " << std::fixed << std::setprecision(2) << daily_average 
              << " km/day vs " << std::setprecision(1) << REQUIRED_DAILY_AVG << " km/day (";
    if (pace_difference >= 0) {
        daily_oss << "+" << std::setprecision(2) << pace_difference << " km/day ahead";
    } else {
        daily_oss << std::setprecision(2) << pace_difference << " km/day behind";
    }
    daily_oss << ")</b>";
    
    goal_oss << "<b>Goal Progress: " << std::fixed << std::setprecision(2) << total 
             << " / " << static_cast<int>(YEARLY_GOAL) << " km (" 
             << std::setprecision(1) << progress_percent << "%)</b>";
    
    m_total_label->setText(QString::fromStdString(total_oss.str()));
    m_average_label->setText(QString::fromStdString(avg_oss.str()));
    m_count_label->setText(QString::fromStdString(count_oss.str()));
    m_daily_avg_label->setText(QString::fromStdString(daily_oss.str()));
    m_goal_label->setText(QString::fromStdString(goal_oss.str()));
}

void MainWindow::save_to_file() {
    std::ofstream file(m_data_file.toStdString());
    
    if (!file.is_open()) {
        QMessageBox::warning(this, "Warning", 
            "Could not save data to file:\n" + m_data_file);
        return;
    }
    
    for (const auto& entry : m_entries) {
        file << entry.date << "," << std::fixed << std::setprecision(2) 
             << entry.kilometers << "\n";
    }
    
    file.close();
}

void MainWindow::load_from_file() {
    std::ifstream file(m_data_file.toStdString());
    
    if (!file.is_open()) {
        // File doesn't exist yet, which is fine for first run
        return;
    }
    
    m_entries.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        size_t comma_pos = line.find(',');
        if (comma_pos == std::string::npos) continue;
        
        std::string date = line.substr(0, comma_pos);
        std::string km_str = line.substr(comma_pos + 1);
        
        try {
            double kilometers = std::stod(km_str);
            m_entries.emplace_back(date, kilometers);
        } catch (const std::exception&) {
            // Skip invalid lines
            continue;
        }
    }
    
    file.close();
}
