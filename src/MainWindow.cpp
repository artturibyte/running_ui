#include "MainWindow.h"
#include <iomanip>
#include <sstream>
#include <ctime>
#include <numeric>
#include <QWidget>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Running Tracker");
    resize(500, 400);
    
    setup_ui();
    update_list_view();
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
    
    QLabel *input_label = new QLabel("Daily Kilometers:", this);
    m_kilometers_entry = new QLineEdit(this);
    m_kilometers_entry->setPlaceholderText("Enter kilometers (e.g., 5.5)");
    m_kilometers_entry->setMaxLength(10);
    
    m_add_button = new QPushButton("Add Entry", this);
    m_clear_button = new QPushButton("Clear All", this);
    
    input_layout->addWidget(input_label);
    input_layout->addWidget(m_kilometers_entry);
    input_layout->addWidget(m_add_button);
    input_layout->addWidget(m_clear_button);
    
    // Statistics section
    QVBoxLayout *stats_layout = new QVBoxLayout();
    stats_layout->setSpacing(5);
    
    m_total_label = new QLabel("<b>Total: 0.0 km</b>", this);
    m_average_label = new QLabel("<b>Average: 0.0 km</b>", this);
    m_count_label = new QLabel("<b>Entries: 0</b>", this);
    
    m_total_label->setTextFormat(Qt::RichText);
    m_average_label->setTextFormat(Qt::RichText);
    m_count_label->setTextFormat(Qt::RichText);
    
    stats_layout->addWidget(m_total_label);
    stats_layout->addWidget(m_average_label);
    stats_layout->addWidget(m_count_label);
    
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
    connect(m_clear_button, &QPushButton::clicked, this, &MainWindow::on_clear_button_clicked);
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
    
    // Add new entry
    m_entries.emplace_back(get_current_date(), kilometers);
    
    // Update UI
    update_list_view();
    update_statistics();
    
    // Clear input field
    m_kilometers_entry->clear();
    m_kilometers_entry->setFocus();
}

void MainWindow::on_clear_button_clicked() {
    if (m_entries.empty()) {
        return;
    }
    
    // Confirmation dialog
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Confirm Clear",
        "Are you sure you want to clear all entries?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        m_entries.clear();
        update_list_view();
        update_statistics();
    }
}

std::string MainWindow::get_current_date() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
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
    if (m_entries.empty()) {
        m_total_label->setText("<b>Total: 0.0 km</b>");
        m_average_label->setText("<b>Average: 0.0 km</b>");
        m_count_label->setText("<b>Entries: 0</b>");
        return;
    }
    
    double total = std::accumulate(m_entries.begin(), m_entries.end(), 0.0,
                                   [](double sum, const RunningEntry& e) {
                                       return sum + e.kilometers;
                                   });
    
    double average = total / m_entries.size();
    
    std::ostringstream total_oss, avg_oss, count_oss;
    total_oss << "<b>Total: " << std::fixed << std::setprecision(2) << total << " km</b>";
    avg_oss << "<b>Average: " << std::fixed << std::setprecision(2) << average << " km</b>";
    count_oss << "<b>Entries: " << m_entries.size() << "</b>";
    
    m_total_label->setText(QString::fromStdString(total_oss.str()));
    m_average_label->setText(QString::fromStdString(avg_oss.str()));
    m_count_label->setText(QString::fromStdString(count_oss.str()));
}
