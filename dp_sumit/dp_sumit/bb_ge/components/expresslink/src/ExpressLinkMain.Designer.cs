namespace ExpressLink
{
    /// <summary>
    /// Required designer variable.
    /// </summary>
    partial class ExpressLink
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            // LogFile.Close();

            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ExpressLink));
            this.labelTitle = new System.Windows.Forms.Label();
            this.labelVersion = new System.Windows.Forms.Label();
            this.labelComPort = new System.Windows.Forms.Label();
            this.comboBoxCOMPort = new System.Windows.Forms.ComboBox();
            this.buttonConnect = new System.Windows.Forms.Button();
            this.buttonUpdateSoftware = new System.Windows.Forms.Button();
            this.buttonHelp = new System.Windows.Forms.Button();
            this.pictureBoxXUSBLogo = new System.Windows.Forms.PictureBox();
            this.labelCurrentVersion = new System.Windows.Forms.Label();
            this.textBoxLogWindow = new System.Windows.Forms.TextBox();
            this.bgWorkerUpdateProgressBar = new System.ComponentModel.BackgroundWorker();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripProgressBar = new System.Windows.Forms.ToolStripProgressBar();
            this.buttonReadSoftware = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxXUSBLogo)).BeginInit();
            this.statusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelTitle
            // 
            this.labelTitle.AutoSize = true;
            this.labelTitle.Font = new System.Drawing.Font("Arial Black", 20.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelTitle.Location = new System.Drawing.Point(186, 9);
            this.labelTitle.Name = "labelTitle";
            this.labelTitle.Size = new System.Drawing.Size(200, 38);
            this.labelTitle.TabIndex = 1;
            this.labelTitle.Text = "ExpressLink";
            // 
            // labelVersion
            // 
            this.labelVersion.AutoSize = true;
            this.labelVersion.Font = new System.Drawing.Font("Arial Black", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelVersion.Location = new System.Drawing.Point(191, 47);
            this.labelVersion.Name = "labelVersion";
            this.labelVersion.Size = new System.Drawing.Size(283, 18);
            this.labelVersion.TabIndex = 1;
            this.labelVersion.Text = "USB Ranger 2000 Series Beta Release";
            // 
            // labelComPort
            // 
            this.labelComPort.AutoSize = true;
            this.labelComPort.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelComPort.Location = new System.Drawing.Point(9, 76);
            this.labelComPort.Name = "labelComPort";
            this.labelComPort.Size = new System.Drawing.Size(65, 16);
            this.labelComPort.TabIndex = 3;
            this.labelComPort.Text = "COM Port";
            // 
            // comboBoxCOMPort
            // 
            this.comboBoxCOMPort.BackColor = System.Drawing.SystemColors.Menu;
            this.comboBoxCOMPort.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxCOMPort.FormattingEnabled = true;
            this.comboBoxCOMPort.Location = new System.Drawing.Point(80, 75);
            this.comboBoxCOMPort.Name = "comboBoxCOMPort";
            this.comboBoxCOMPort.Size = new System.Drawing.Size(80, 21);
            this.comboBoxCOMPort.Sorted = true;
            this.comboBoxCOMPort.TabIndex = 1;
            // 
            // buttonConnect
            // 
            this.buttonConnect.BackColor = System.Drawing.SystemColors.Window;
            this.buttonConnect.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.buttonConnect.Location = new System.Drawing.Point(12, 102);
            this.buttonConnect.Name = "buttonConnect";
            this.buttonConnect.Size = new System.Drawing.Size(180, 60);
            this.buttonConnect.TabIndex = 2;
            this.buttonConnect.Text = "Connect";
            this.buttonConnect.UseVisualStyleBackColor = false;
            this.buttonConnect.Click += new System.EventHandler(this.buttonConnect_Click);
            // 
            // buttonUpdateSoftware
            // 
            this.buttonUpdateSoftware.BackColor = System.Drawing.SystemColors.Window;
            this.buttonUpdateSoftware.Enabled = false;
            this.buttonUpdateSoftware.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.buttonUpdateSoftware.Location = new System.Drawing.Point(198, 102);
            this.buttonUpdateSoftware.Name = "buttonUpdateSoftware";
            this.buttonUpdateSoftware.Size = new System.Drawing.Size(180, 60);
            this.buttonUpdateSoftware.TabIndex = 3;
            this.buttonUpdateSoftware.Text = "Update Software";
            this.buttonUpdateSoftware.UseVisualStyleBackColor = false;
            this.buttonUpdateSoftware.Click += new System.EventHandler(this.buttonUpdateSoftware_Click);
            // 
            // buttonHelp
            // 
            this.buttonHelp.AutoSize = true;
            this.buttonHelp.Location = new System.Drawing.Point(502, 73);
            this.buttonHelp.Name = "buttonHelp";
            this.buttonHelp.Size = new System.Drawing.Size(61, 23);
            this.buttonHelp.TabIndex = 6;
            this.buttonHelp.Text = "Help";
            this.buttonHelp.UseVisualStyleBackColor = true;
            this.buttonHelp.Click += new System.EventHandler(this.buttonHelp_Click);
            // 
            // pictureBoxXUSBLogo
            // 
            this.pictureBoxXUSBLogo.Image = ((System.Drawing.Image)(resources.GetObject("pictureBoxXUSBLogo.Image")));
            this.pictureBoxXUSBLogo.Location = new System.Drawing.Point(12, 9);
            this.pictureBoxXUSBLogo.Name = "pictureBoxXUSBLogo";
            this.pictureBoxXUSBLogo.Size = new System.Drawing.Size(142, 56);
            this.pictureBoxXUSBLogo.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBoxXUSBLogo.TabIndex = 9;
            this.pictureBoxXUSBLogo.TabStop = false;
            // 
            // labelCurrentVersion
            // 
            this.labelCurrentVersion.AutoSize = true;
            this.labelCurrentVersion.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelCurrentVersion.Location = new System.Drawing.Point(415, 165);
            this.labelCurrentVersion.Name = "labelCurrentVersion";
            this.labelCurrentVersion.Size = new System.Drawing.Size(0, 16);
            this.labelCurrentVersion.TabIndex = 10;
            // 
            // textBoxLogWindow
            // 
            this.textBoxLogWindow.Location = new System.Drawing.Point(12, 168);
            this.textBoxLogWindow.Multiline = true;
            this.textBoxLogWindow.Name = "textBoxLogWindow";
            this.textBoxLogWindow.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxLogWindow.Size = new System.Drawing.Size(458, 122);
            this.textBoxLogWindow.TabIndex = 12;
            this.textBoxLogWindow.TabStop = false;
            this.textBoxLogWindow.Visible = false;
            // 
            // bgWorkerUpdateProgressBar
            // 
            this.bgWorkerUpdateProgressBar.WorkerSupportsCancellation = true;
            this.bgWorkerUpdateProgressBar.DoWork += new System.ComponentModel.DoWorkEventHandler(this.bgWorkerUpdateProgressBar_DoWork);
            // 
            // statusStrip
            // 
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel,
            this.toolStripProgressBar});
            this.statusStrip.Location = new System.Drawing.Point(0, 166);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Size = new System.Drawing.Size(575, 22);
            this.statusStrip.SizingGrip = false;
            this.statusStrip.TabIndex = 14;
            this.statusStrip.Text = "statusStrip1";
            // 
            // toolStripStatusLabel
            // 
            this.toolStripStatusLabel.BackColor = System.Drawing.SystemColors.Control;
            this.toolStripStatusLabel.Name = "toolStripStatusLabel";
            this.toolStripStatusLabel.Size = new System.Drawing.Size(149, 17);
            this.toolStripStatusLabel.Text = "";
            // 
            // toolStripProgressBar
            // 
            this.toolStripProgressBar.Name = "toolStripProgressBar";
            this.toolStripProgressBar.Size = new System.Drawing.Size(391, 16);
            this.toolStripProgressBar.ToolTipText = "Download Progress";
            this.toolStripProgressBar.Visible = false;
            // 
            // buttonReadSoftware
            // 
            this.buttonReadSoftware.BackColor = System.Drawing.SystemColors.Window;
            this.buttonReadSoftware.Enabled = false;
            this.buttonReadSoftware.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.buttonReadSoftware.Location = new System.Drawing.Point(384, 102);
            this.buttonReadSoftware.Name = "buttonReadSoftware";
            this.buttonReadSoftware.Size = new System.Drawing.Size(180, 60);
            this.buttonReadSoftware.TabIndex = 15;
            this.buttonReadSoftware.Text = "Read Software";
            this.buttonReadSoftware.UseVisualStyleBackColor = false;
            this.buttonReadSoftware.Click += new System.EventHandler(this.buttonReadSoftware_Click);
            // 
            // ExpressLink
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.ClientSize = new System.Drawing.Size(575, 188);
            this.Controls.Add(this.buttonReadSoftware);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.textBoxLogWindow);
            this.Controls.Add(this.labelCurrentVersion);
            this.Controls.Add(this.buttonHelp);
            this.Controls.Add(this.buttonUpdateSoftware);
            this.Controls.Add(this.buttonConnect);
            this.Controls.Add(this.comboBoxCOMPort);
            this.Controls.Add(this.labelComPort);
            this.Controls.Add(this.labelVersion);
            this.Controls.Add(this.labelTitle);
            this.Controls.Add(this.pictureBoxXUSBLogo);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "ExpressLink";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "ExpressLink";
            this.Load += new System.EventHandler(this.ExpressLink_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxXUSBLogo)).EndInit();
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelTitle;
        private System.Windows.Forms.Label labelVersion;
        private System.Windows.Forms.Label labelComPort;
        private System.Windows.Forms.ComboBox comboBoxCOMPort;
        private System.Windows.Forms.Button buttonConnect;
        private System.Windows.Forms.Button buttonUpdateSoftware;
        private System.Windows.Forms.Button buttonHelp;
        private System.Windows.Forms.PictureBox pictureBoxXUSBLogo;
        private System.Windows.Forms.Label labelCurrentVersion;
        private System.Windows.Forms.TextBox textBoxLogWindow;
        private System.ComponentModel.BackgroundWorker bgWorkerUpdateProgressBar;
        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel;
        private System.Windows.Forms.ToolStripProgressBar toolStripProgressBar;

        public void updateLabelWithReleaseInfo(string newLabelString)
        {
            this.labelVersion.Text = newLabelString;
        }

        private System.Windows.Forms.Button buttonReadSoftware;
    }
}

