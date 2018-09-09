const axios = require('axios')
const nodemailer = require('nodemailer')
const transporter = nodemailer.createTransport({
    host: 'smtp.gmail.com',
    port: 587,
    secure: false,
    auth: {
        user: 'somebody@gmail.com',  // your email
        pass: 'password' // your mail password
    }
})

const toEmail = 'somebody@gmail.com' // where to send email
const fromEmail = '"DROID" <somebody@gmail.com>' // your mail
const nodeIpAddress = '109.92.6.40' // IP OF YOUR HN
const interval = 300 // how often to check your HN status

function sendMail(status) {
    let mailOptions = {
        from: fromEmail,
        to: toEmail,
        subject: `BISMUTH NODE ALERT`,
        text: `STATUS: ${status}`,
        html: `<b>STATUS: ${status}</b>`
    }
    transporter.sendMail(mailOptions, (error, info) => {
        if (error) { console.log(error) }
        else { console.log(info) }
    })
}

function start() {
    axios.get(`https://hypernodes.bismuth.live/status.json`)
        .then((response) => {
            let index = Object.keys(response.data).findIndex((element) => { return element === nodeIpAddress })

            if (index > -1) {
                let status = response.data[nodeIpAddress]
                if (status === -1) {
                    console.log('[ OFFLINE ]')
                    sendMail(status)
                } else {
                    console.log('[ ONLINE ]')
                }
            }
        })
        .catch((error) => { console.log(error) })

    setTimeout(() => { start() },  interval * 1000)
}

start()